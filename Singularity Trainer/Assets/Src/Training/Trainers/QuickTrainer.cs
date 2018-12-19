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
using MessagePack;

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
        private List<float> EpisodeRewards { get; set; }

        private void Awake()
        {
            EnvironmentContexts = new Dictionary<IEnvironment, int>();
            ObservationQueue = new List<IObservation>();
            var environments = GetComponentsInChildren<IEnvironment>().ToList();
            EpisodeRewards = environments.Select(x => 0f).ToList();
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

                var beginSessionRequest = new BeginSessionRequest
                {
                    Param = new BeginSessionParams
                    {
                        Model = new ModelParams
                        {
                            Inputs = 18,
                            Outputs = 4,
                            Recurrent = false,
                            NormalizeInputs = true
                        },
                        Hyperparams = new HyperParams
                        {
                            LearningRate = 0.00004f,
                            Gae = 0.95f,
                            BatchSize = 256,
                            NumMinibatch = 8,
                            EntropyCoef = 0.00001f,
                            MaxGradNorm = 0.5f,
                            DiscountFactor = 0.95f,
                            CriticCoef = 0.5f,
                            Epochs = 3,
                            ClipFactor = 0.2f,
                            NormalizeRewards = true
                        },
                        SessionId = 0,
                        Training = true,
                        Contexts = 8
                    }
                };

                client.TrySendFrame(waitTime, MessagePackSerializer.Serialize<BeginSessionRequest>(beginSessionRequest));

                client.TryReceiveFrameString(waitTime, out string receivedMessage);
                Debug.Log(receivedMessage);
            }
            catch (Exception ex)
            {
                Debug.LogError(ex);
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
                Param = new GetActionParam
                {
                    Inputs = ObservationQueue.Select(o => o.ToList()).ToList(),
                    SessionId = 0
                },
                Id = 0
            };

            client.TrySendFrame(waitTime, MessagePackSerializer.Serialize<GetActionRequest>(getActionRequest));

            client.TryReceiveFrameBytes(waitTime, out byte[] receivedMessage);

            var getActionResponse = MessagePackSerializer.Deserialize<GetActionResponse>(receivedMessage);
            var rewards = new List<float>();
            var dones = new List<int>();
            for (int i = 0; i < ObservationQueue.Count; i++)
            {
                var observation = ObservationQueue[i];
                observation.Environment.SetValue(observation.AgentNumber, getActionResponse.Result.Value[i]);
                observation.Environment.SendActions(observation.AgentNumber, getActionResponse.Result.Actions[i]);

                var rewardAndDone = observation.Environment.GetReward(observation.AgentNumber);
                float reward = rewardAndDone.Item1;
                int done = rewardAndDone.Item2;
                rewards.Add(reward);
                dones.Add(done);
                EpisodeRewards[i] += reward;
                if (done == 1)
                {
                    RewardChart.AddDataPoint(EpisodeRewards[i]);
                    EpisodeRewards[i] = 0;
                }
            }
            if (RewardChart.SmoothedData.Count > 0)
            {
                rewardText.SetText(RewardChart.SmoothedData.Last().ToString("F2"));
            }

            var giveRewardRequest = new GiveRewardRequest
            {
                Param = new GiveRewardParams
                {
                    Rewards = rewards,
                    Dones = dones
                },
                Id = 0
            };

            client.TrySendFrame(waitTime, MessagePackSerializer.Serialize<GiveRewardRequest>(giveRewardRequest));
            client.ReceiveFrameBytes();

            ObservationQueue.Clear();
        }
    }

    [MessagePackObject]
    abstract class Request
    {
        [Key("api")]
        public string Api { get; set; } = "v1alpha1";

        [Key("method")]
        public string Method { get; set; }

        [Key("id")]
        public int Id { get; set; } = 0;
    }

    [MessagePackObject]
    class GetActionRequest : Request
    {
        [Key("param")]
        public GetActionParam Param { get; set; }

        public GetActionRequest()
        {
            Method = "get_action";
        }
    }

    [MessagePackObject]
    class GetActionParam
    {
        [Key("inputs")]
        public List<List<float>> Inputs { get; set; }

        [Key("sessionId")]
        public int SessionId { get; set; }
    }

    [MessagePackObject]
    class GiveRewardRequest : Request
    {
        [Key("param")]
        public GiveRewardParams Param { get; set; }

        public GiveRewardRequest()
        {
            Method = "give_reward";
        }
    }

    [MessagePackObject]
    class GiveRewardParams
    {
        [Key("rewards")]
        public List<float> Rewards { get; set; }

        [Key("dones")]
        public List<int> Dones { get; set; }

        [Key("sessionId")]
        public int SessionId { get; set; }
    }

    [MessagePackObject]
    abstract class Response
    {
        [Key("api")]
        public string Api { get; set; }

        [Key("id")]
        public int Id { get; set; }
    }

    [MessagePackObject]
    class GetActionResponse : Response
    {
        [Key("result")]
        public GetActionResult Result { get; set; }
    }

    [MessagePackObject]
    class GetActionResult
    {
        [Key("actions")]
        public List<List<bool>> Actions { get; set; }

        [Key("value")]
        public List<float> Value { get; set; }
    }

    [MessagePackObject]
    class BeginSessionRequest
    {
        [Key("param")]
        public BeginSessionParams Param { get; set; }
    }

    [MessagePackObject]
    class BeginSessionParams
    {
        [Key("model")]
        public ModelParams Model { get; set; }

        [Key("hyperparams")]
        public HyperParams Hyperparams { get; set; }

        [Key("session_id")]
        public int SessionId { get; set; }

        [Key("training")]
        public bool Training { get; set; }

        [Key("contexts")]
        public int Contexts { get; set; }
    }

    [MessagePackObject]
    class ModelParams
    {
        [Key("inputs")]
        public int Inputs { get; set; }

        [Key("outputs")]
        public int Outputs { get; set; }

        [Key("recurrent")]
        public bool Recurrent { get; set; }

        [Key("normalize_inputs")]
        public bool NormalizeInputs { get; set; }
    }

    [MessagePackObject]
    class HyperParams
    {
        [Key("learning_rate")]
        public float LearningRate { get; set; }

        [Key("gae")]
        public float Gae { get; set; }

        [Key("batch_size")]
        public int BatchSize { get; set; }

        [Key("num_minibatch")]
        public int NumMinibatch { get; set; }

        [Key("entropy_coef")]
        public float EntropyCoef { get; set; }

        [Key("max_grad_norm")]
        public float MaxGradNorm { get; set; }

        [Key("discount_factor")]
        public float DiscountFactor { get; set; }

        [Key("critic_coef")]
        public float CriticCoef { get; set; }

        [Key("epochs")]
        public int Epochs { get; set; }

        [Key("clip_factor")]
        public float ClipFactor { get; set; }

        [Key("normalize_rewards")]
        public bool NormalizeRewards { get; set; }
    }
}
