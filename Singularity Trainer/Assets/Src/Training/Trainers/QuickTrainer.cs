using System.Collections.Generic;
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
        public float averageLength = 1000;
        public List<IObservation> ObservationQueue { get; set; }

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

        private void CleanUp()
        {
            var endSessionRequest = new JObject
            {
                ["jsonrpc"] = "2.0",
                ["method"] = "end_session",
                ["param"] = new JObject
                {
                    ["session_id"] = 0
                },
                ["id"] = 0
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
                    ["jsonrpc"] = "2.0",
                    ["method"] = "begin_session",
                    ["param"] = new JObject
                    {
                        ["model"] = new JObject
                        {
                            ["inputs"] = 18,
                            ["outputs"] = 4,
                            ["recurrent"] = true,
                            ["normalize_rewards"] = true
                        },
                        ["hyperparams"] = new JObject
                        {
                            ["learning_rate"] = 0.0007,
                            ["gae"] = 0.95,
                            ["batch_size"] = 2048,
                            ["num_minibatch"] = 8,
                            ["entropy_coef"] = 0.001,
                            ["max_grad_norm"] = 0.5,
                            ["discount_factor"] = 0.9,
                            ["critic_coef"] = 0.5,
                            ["epochs"] = 4,
                            ["clip_factor"] = 0.1,
                            ["normalize_rewards"] = true
                        },
                        ["session_id"] = 0,
                        ["training"] = true,
                        ["contexts"] = 8
                    },
                    ["id"] = 0
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

            var getActionRequest = new GetActionRequest()
            {
                Inputs = ObservationQueue.Select(o => o.ToList()).ToList()
            };

            client.TrySendFrame(waitTime, getActionRequest.ToJson());

            client.TryReceiveFrameString(waitTime, out string receivedMessage);

            var actionMessage = JObject.Parse(receivedMessage);
            List<List<bool>> actions = actionMessage["result"]["actions"].ToObject<List<List<bool>>>();
            List<float> values = actionMessage["result"]["value"].ToObject<List<float>>();
            var rewards = new List<float>();
            var dones = new List<int>();
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

            client.TrySendFrame(waitTime, giveRewardRequest.ToJson());
            client.ReceiveFrameString();

            ObservationQueue.Clear();
        }
    }

    class GetActionRequest
    {
        public List<List<float>> Inputs { get; set; }

        public string ToJson()
        {
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.Append("{\"jsonrpc\":\"2.0\",\"method\":\"get_actions\",\"param\":{\"inputs\":[");
            foreach (List<float> agent in Inputs)
            {
                stringBuilder.Append("[");
                stringBuilder.Append(String.Join(",", agent));
                stringBuilder.Append("]");
            }
            stringBuilder.Append("],\"session_id\":0},\"id\":0}");

            return stringBuilder.ToString();
        }
    }


    class GiveRewardRequest
    {
        public List<float> Rewards { get; set; }
        public List<int> Dones { get; set; }

        public string ToJson()
        {
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.Append("{\"jsonrpc\":\"2.0\",\"method\":\"give_rewards\",\"param\":{\"reward\":[");
            stringBuilder.Append(String.Join(",", Rewards));
            stringBuilder.Append("],\"done\":[");
            stringBuilder.Append(String.Join(",", Dones));
            stringBuilder.Append("],\"session_id\":0},\"id\":0}");

            return stringBuilder.ToString();
        }
    }
}
