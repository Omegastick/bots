﻿using System.Collections.Generic;
using System.Linq;
using NetMQ;
using Newtonsoft.Json.Linq;
using TMPro;
using Training.Environments;
using UnityEngine;
using Observations;
using Scripts;

namespace Training.Trainers
{
    public class QuickTrainer : MonoBehaviour, ITrainer
    {

        public TextMeshProUGUI rewardText;
        public Queue<IObservation> ObservationQueue { get; set; }
        public float averageLength = 1000;

        private NetMQ.Sockets.PairSocket client;
        private readonly System.TimeSpan waitTime = new System.TimeSpan(0, 0, 0, 10);
        private Dictionary<IEnvironment, int> EnvironmentContexts { get; set; }
        private float AverageReward { get; set; }
        private Chart RewardChart { get; set; }

        private void Awake()
        {
            EnvironmentContexts = new Dictionary<IEnvironment, int>();
            AverageReward = 0f;
            ObservationQueue = new Queue<IObservation>();
            var environments = GetComponentsInChildren<IEnvironment>().ToList();
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
                ["jsonrpc"] = "2.0",
                ["method"] = "give_reward",
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
                            ["inputs"] = new JArray { 21 },
                            ["outputs"] = new JArray { 2, 2, 2, 2 },
                            ["feature_extractors"] = new JArray() { "mlp" },
                            ["recurrent"] = true
                        },
                        ["hyperparams"] = new JObject
                        {
                            ["learning_rate"] = 0.0003,
                            ["gae"] = 0.95,
                            ["batch_size"] = 2400,
                            ["minibatch_length"] = 10,
                            ["entropy_coef"] = 0.0001,
                            ["max_grad_norm"] = 0.5,
                            ["discount_factor"] = 0.92,
                            ["critic_coef"] = 0.5,
                            ["epochs"] = 5,
                            ["clip_factor"] = 0.2
                        },
                        ["session_id"] = 0,
                        ["training"] = true,
                        ["contexts"] = 4,
                        ["auto_train"] = true
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
            foreach (var observation in ObservationQueue)
            {
                var environment = observation.Environment;

                var getActionRequest = new JObject
                {
                    ["jsonrpc"] = "2.0",
                    ["method"] = "get_action",
                    ["param"] = new JObject
                    {
                        ["inputs"] = new JArray { new JArray(observation.ToArray()) },
                        ["context"] = EnvironmentContexts[observation.Environment],
                        ["session_id"] = 0
                    },
                    ["id"] = 0
                };

                var x = getActionRequest.ToString();
                client.TrySendFrame(waitTime, getActionRequest.ToString());

                client.TryReceiveFrameString(waitTime, out string receivedMessage);

                var actionMessage = JObject.Parse(receivedMessage);
                List<int> actions = actionMessage["result"]["actions"].ToObject<List<int>>();
                float value = actionMessage["result"]["value"].ToObject<float>();
                observation.Environment.SetValue(observation.AgentNumber, value);
                observation.Environment.SendActions(observation.AgentNumber, actions);

                var reward = observation.Environment.GetReward(observation.AgentNumber);
                AverageReward -= AverageReward / averageLength;
                AverageReward += reward / averageLength;
                rewardText.SetText(AverageReward.ToString());
                RewardChart.AddDataPoint(AverageReward);

                var giveRewardRequest = new JObject
                {
                    ["jsonrpc"] = "2.0",
                    ["method"] = "give_reward",
                    ["param"] = new JObject
                    {
                        ["reward"] = reward,
                        ["context"] = EnvironmentContexts[observation.Environment],
                        ["session_id"] = 0
                    },
                    ["id"] = 0
                };

                client.TrySendFrame(waitTime, giveRewardRequest.ToString());
                client.ReceiveFrameString();
            }
            ObservationQueue.Clear();
        }
    }
}