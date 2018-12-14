﻿using System.Collections.Generic;
using System.IO;
using System.Linq;
using NetMQ;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json.Serialization;
using Observations;
using Scripts;
using TMPro;
using Training.Environments;
using UnityEngine;

namespace Training.Trainers
{
    public class QuickTrainer : MonoBehaviour, ITrainer
    {

        public TextMeshProUGUI rewardText;
        public List<IObservation> ObservationQueue { get; set; }
        public float averageLength = 1000;

        private NetMQ.Sockets.PairSocket client;
        private readonly System.TimeSpan waitTime = new System.TimeSpan(0, 0, 0, 20);
        private Dictionary<IEnvironment, int> EnvironmentContexts { get; set; }
        private float AverageReward { get; set; }
        private Chart RewardChart { get; set; }
        private int EnvironmentCount { get; set; }

        private void Awake()
        {
            EnvironmentContexts = new Dictionary<IEnvironment, int>();
            AverageReward = 0f;
            ObservationQueue = new List<IObservation>();
            var environments = GetComponentsInChildren<IEnvironment>().ToList();
            EnvironmentCount = environments.Count;
            for (int i = 0; i < environments.Count; i++)
            {
                environments[i].Trainer = this;
                EnvironmentContexts.Add(environments[i], i);
            }
            RewardChart = GetComponentInChildren<Chart>();
        }

        private void OnApplicationQuit()
        {
            CleanUp();
        }

        private void OnDestroy()
        {
            CleanUp();
        }

        private void CleanUp()
        {
            var endSessionRequest = new JObject
            {
                ["jsonrpc"] = "2.0", ["method"] = "end_session", ["param"] = new JObject
                {
                ["session_id"] = 0
                }, ["id"] = 0
            };
            client.TrySendFrame(waitTime, endSessionRequest.ToString());
            NetMQConfig.Cleanup(false);
        }

        public void BeginTraining()
        {
            try
            {
                AsyncIO.ForceDotNet.Force();

                client = new NetMQ.Sockets.PairSocket();
                client.Connect("tcp://127.0.0.1:10201");

                var message = client.ReceiveFrameString();
                Debug.Log(message);

                client.TrySendFrame(waitTime, "Connection established...");

                var sessionRequest = new JObject
                {
                    ["jsonrpc"] = "2.0", ["method"] = "begin_session", ["param"] = new JObject
                    {
                    ["model"] = new JObject
                    {
                    ["inputs"] = 18, ["outputs"] = 4, ["recurrent"] = true, ["normalize_rewards"] = true
                    }, ["hyperparams"] = new JObject
                    {
                    ["learning_rate"] = 0.0007, ["gae"] = 0.95, ["batch_size"] = 2048, ["num_minibatch"] = 8, ["entropy_coef"] = 0.001, ["max_grad_norm"] = 0.5, ["discount_factor"] = 0.9, ["critic_coef"] = 0.5, ["epochs"] = 4, ["clip_factor"] = 0.1, ["normalize_rewards"] = true
                    }, ["session_id"] = 0, ["training"] = true, ["contexts"] = 8
                    }, ["id"] = 0
                };

                client.TrySendFrame(waitTime, sessionRequest.ToString());

                client.TryReceiveFrameString(waitTime, out string receivedMessage);
                Debug.Log(receivedMessage);
            }
            catch
            {
                CleanUp();
            }
        }

        public void EndTraining()
        {
            CleanUp();
        }

        public void SaveModel(string path)
        {
            throw new System.NotImplementedException();
        }

        public void Step()
        {
            if (ObservationQueue.Count < EnvironmentCount)
            {
                return;
            }

            ObservationQueue = ObservationQueue.OrderBy(o => EnvironmentContexts[o.Environment]).ToList();

            DefaultContractResolver snakeCaseContractResolver = new DefaultContractResolver
            {
                NamingStrategy = new SnakeCaseNamingStrategy()
            };

            var getActionRequest = new GetActionRequest()
            {
                Inputs = ObservationQueue.Select(o => o.ToList()).ToList()
            };

            // var getActionRequest = new Request
            // {
            //     Jsonrpc = "2.0",
            //     Method = "get_actions",
            //     Param = new GetActionParam
            //     {
            //     Inputs = ObservationQueue.Select(o => o.ToList()).ToList(),
            //     SessionId = 0
            //     },
            //     Id = 0
            // };

            // var json = JsonConvert.SerializeObject(getActionRequest, new JsonSerializerSettings
            // {
            //     ContractResolver = snakeCaseContractResolver,
            //         Formatting = Formatting.None
            // });
            client.TrySendFrame(waitTime, getActionRequest.ToJson());

            client.TryReceiveFrameString(waitTime, out string receivedMessage);

            var actionMessage = JObject.Parse(receivedMessage);
            List<List<bool>> actions = actionMessage["result"]["actions"].ToObject<List<List<bool>>>();
            List<float> values = actionMessage["result"]["value"].ToObject<List<float>>();
            var rewards = new List<float>();
            var dones = new List<bool>();
            for (int i = 0; i < ObservationQueue.Count; i++)
            {
                var observation = ObservationQueue[i];
                observation.Environment.SetValue(observation.AgentNumber, values[i]);
                observation.Environment.SendActions(observation.AgentNumber, actions[i]);

                var rewardAndDone = observation.Environment.GetReward(observation.AgentNumber);
                float reward = rewardAndDone.Item1;
                rewards.Add(reward);
                dones.Add(rewardAndDone.Item2);
                AverageReward -= AverageReward / averageLength;
                AverageReward += reward / averageLength;
            }
            rewardText.SetText(AverageReward.ToString());
            RewardChart.AddDataPoint(AverageReward);

            var giveRewardRequest = new GiveRewardRequest()
            {
                Rewards = rewards,
                    Dones = dones
            };

            // var giveRewardRequest = new Request
            // {
            //     Jsonrpc = "2.0",
            //     Method = "give_rewards",
            //     Param = new GiveRewardParam
            //     {
            //     Reward = rewards,
            //     Done = dones,
            //     SessionId = 0
            //     },
            //     Id = 0
            // };

            // var json = JsonConvert.SerializeObject(giveRewardRequest, new JsonSerializerSettings
            // {
            //     ContractResolver = snakeCaseContractResolver,
            //         Formatting = Formatting.None
            // });
            client.TrySendFrame(waitTime, giveRewardRequest.ToJson());
            client.ReceiveFrameString();

            ObservationQueue.Clear();
        }
    }

    class Request
    {
        public string Jsonrpc { get; set; }
        public string Method { get; set; }
        public object Param { get; set; }
        public int Id { get; set; }
    }

    class GetActionParam
    {
        public List<List<float>> Inputs { get; set; }
        public int SessionId;
    }

    class GiveRewardParam
    {
        public List<float> Reward;
        public List<bool> Done;
        public int SessionId;
    }

    class GetActionRequest
    {
        public List<List<float>> Inputs { get; set; }
        public string ToJson()
        {
            StringWriter sw = new StringWriter();
            JsonTextWriter writer = new JsonTextWriter(sw);

            writer.WriteStartObject();

            writer.WritePropertyName("jsonrpc");
            writer.WriteValue("2.0");

            writer.WritePropertyName("method");
            writer.WriteValue("get_actions");

            writer.WritePropertyName("param");
            writer.WriteStartObject();

            writer.WritePropertyName("inputs");
            writer.WriteStartArray();
            for (int i = 0; i < this.Inputs.Count; i++)
            {
                List<float> inputs = this.Inputs[i];
                writer.WriteStartArray();
                for (int j = 0; j < inputs.Count; j++)
                {
                    writer.WriteValue(inputs[j]);
                }
                writer.WriteEndArray();
            }
            writer.WriteEndArray();

            writer.WritePropertyName("session_id");
            writer.WriteValue(0);

            writer.WriteEndObject();

            writer.WritePropertyName("id");
            writer.WriteValue(0);

            writer.WriteEndObject();

            return sw.ToString();
        }
    }

    class GiveRewardRequest
    {
        public List<float> Rewards { get; set; }
        public List<bool> Dones { get; set; }

        public string ToJson()
        {
            StringWriter sw = new StringWriter();
            JsonTextWriter writer = new JsonTextWriter(sw);

            writer.WriteStartObject();

            writer.WritePropertyName("jsonrpc");
            writer.WriteValue("2.0");

            writer.WritePropertyName("method");
            writer.WriteValue("give_rewards");

            writer.WritePropertyName("param");
            writer.WriteStartObject();

            writer.WritePropertyName("reward");
            writer.WriteStartArray();
            for (int i = 0; i < this.Rewards.Count; i++)
            {
                writer.WriteValue(this.Rewards[i]);
            }
            writer.WriteEndArray();

            writer.WritePropertyName("done");
            writer.WriteStartArray();
            for (int i = 0; i < this.Rewards.Count; i++)
            {
                writer.WriteValue(this.Dones[i] ? 1 : 0);
            }
            writer.WriteEndArray();

            writer.WritePropertyName("session_id");
            writer.WriteValue(0);

            writer.WriteEndObject();

            writer.WritePropertyName("id");
            writer.WriteValue(0);

            writer.WriteEndObject();

            return sw.ToString();
        }
    }
}
