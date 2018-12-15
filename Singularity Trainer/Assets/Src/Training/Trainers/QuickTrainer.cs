using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using NetMQ;
using Observations;
using Scripts;
using TMPro;
using Training.Environments;
using UnityEngine;
using SimpleJSON;

namespace Training.Trainers
{
    public class QuickTrainer : MonoBehaviour, ITrainer
    {

        public TextMeshProUGUI rewardText;
        public List<IObservation> ObservationQueue { get; set; }

        private NetMQ.Sockets.PairSocket client;
        private readonly System.TimeSpan waitTime = new System.TimeSpan(0, 0, 0, 1);
        private Dictionary<IEnvironment, int> EnvironmentContexts { get; set; }
        private Chart RewardChart { get; set; }
        private int EnvironmentCount { get; set; }

        private void Awake()
        {
            EnvironmentContexts = new Dictionary<IEnvironment, int>();
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
            string endSessionRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"end_session\",\"param\":{\"session_id\":0},\"id\":0}";
            client.TrySendFrame(waitTime, endSessionRequest);
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

                var sessionRequest = @"
                {
                    ""jsonrpc"": ""2.0"",
                    ""method"": ""begin_session"",
                    ""param"":
                    {
                        ""model"":
                        {
                            ""inputs"": 18,
                            ""outputs"": 4,
                            ""recurrent"": true,
                            ""normalize_rewards"": true
                        },
                        ""hyperparams"":
                        {
                            ""learning_rate"": 0.001,
                            ""gae"": 0.95,
                            ""batch_size"": 4096,
                            ""num_minibatch"": 8,
                            ""entropy_coef"": 0.001,
                            ""max_grad_norm"": 0.5,
                            ""discount_factor"": 0.95,
                            ""critic_coef"": 0.5,
                            ""epochs"": 4,
                            ""clip_factor"": 0.2,
                            ""normalize_rewards"": true
                        },
                        ""session_id"": 0,
                        ""training"": true,
                        ""contexts"": 8
                    },
                    ""id"": 0
                }";

                client.TrySendFrame(waitTime, sessionRequest);

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

            var actionMessage = JSON.Parse(receivedMessage);
            List<List<bool>> actions = new List<List<bool>>();
            foreach (JSONArray agent in actionMessage["result"]["actions"])
            {
                List<bool> agentActions = new List<bool>();
                foreach (JSONNode action in agent) {
                    agentActions.Add(action.AsBool);
                }
                actions.Add(agentActions);
            }
            List<float> values = new List<float>();
            foreach (JSONNode action in actionMessage["result"]["value"]) {
                values.Add(action.AsFloat);
            }
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
                RewardChart.AddDataPoint(reward);
            }
            if (RewardChart.SmoothedData.Count > 0) {
                rewardText.SetText(RewardChart.SmoothedData.Last().ToString("F2"));
            }

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
                stringBuilder.Append("],");
            }
            stringBuilder.Remove(stringBuilder.Length - 1, 1);
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
