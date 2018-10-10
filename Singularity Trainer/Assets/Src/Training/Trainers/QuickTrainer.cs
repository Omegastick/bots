using NetMQ;
using Newtonsoft.Json.Linq;
using Observations;
using System.Collections.Generic;
using Training.Environments;
using UnityEngine;

namespace Training.Trainers
{
    public class QuickTrainer : MonoBehaviour, ITrainer
    {

        public UnityEngine.UI.Text rewardText;
        private NetMQ.Sockets.PairSocket client;
        private readonly System.TimeSpan waitTime = new System.TimeSpan(0, 0, 0, 1);
        private IEnvironment[] environments;

        public Queue<IObservation> ObservationQueue { get; set; }


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
            environments = GetComponentsInChildren<IEnvironment>();
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
                            ["outputs"] = new JArray { 4 },
                            ["feature_extractors"] = new JArray() { "mlp" }
                        },
                        ["hyperparams"] = new JObject
                        {
                            ["learning_rate"] = 0.0007,
                            ["gae"] = 0.95,
                            ["batch_size"] = 50,
                            ["minibatch_length"] = 5,
                            ["minibatch_count"] = 10,
                            ["entropy_coef"] = 0.0001,
                            ["max_grad_norm"] = 0.5,
                            ["discount_factor"] = 0.92,
                            ["critic_coef"] = 0.2,
                            ["epochs"] = 4,
                            ["clip_factor"] = 0.2
                        },
                        ["session_id"] = 0,
                        ["training"] = true,
                        ["contexts"] = 1,
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
                        ["context"] = observation.AgentNumber,
                        ["session_id"] = 0
                    },
                    ["id"] = 0
                };

                client.TrySendFrame(waitTime, getActionRequest.ToString());

                client.TryReceiveFrameString(waitTime, out string receivedMessage);

                var actionMessage = JObject.Parse(receivedMessage);
                List<int> actions = actionMessage["result"]["actions"].ToObject<List<int>>();
                float value = actionMessage["result"]["value"].ToObject<float>();
                observation.Environment.SendActions(observation.AgentNumber, actions);

                var reward = observation.Environment.GetReward(observation.AgentNumber);

                var giveRewardRequest = new JObject
                {
                    ["jsonrpc"] = "2.0",
                    ["method"] = "give_reward",
                    ["param"] = new JObject
                    {
                        ["reward"] = reward,
                        ["context"] = observation.AgentNumber,
                        ["session_id"] = 0
                    },
                    ["id"] = 0
                };

                client.SendFrame(giveRewardRequest.ToString());
                client.TryReceiveFrameString(waitTime, out receivedMessage);
            }
            ObservationQueue.Clear();
        }
    }
}