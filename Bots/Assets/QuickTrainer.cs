using System.Collections.Generic;
using System.Linq;
using NetMQ;
using Newtonsoft.Json.Linq;
using UnityEngine;

public class QuickTrainer : MonoBehaviour
{

    public UnityEngine.UI.Text rewardText;
    public float timeScale = 1;
    private NetMQ.Sockets.PairSocket client;
    private float lastActionTime;
    private List<float> rewards = new List<float>();
    private System.TimeSpan waitTime = new System.TimeSpan(0, 0, 0, 500);
    private Environment[] environments;

    private void Start()
    {
        environments = FindObjectsOfType<Environment>();
        lastActionTime = Time.time;
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
                        ["inputs"] = new JArray { 7 },
                        ["outputs"] = new JArray { 5 },
                        ["feature_extractors"] = new JArray() { "mlp" }
                    },
                    ["hyperparams"] = new JObject
                    {
                        ["learning_rate"] = 0.0007,
                        ["gae"] = 0.6,
                        ["batch_size"] = 250,
                        ["minibatch_length"] = 5,
                        ["minibatch_count"] = 60,
                        ["entropy_coef"] = 0.0001,
                        ["max_grad_norm"] = 0.5,
                        ["discount_factor"] = 0.98,
                        ["critic_coef"] = 0.5,
                        ["epochs"] = 10
                    },
                    ["session_id"] = 0,
                    ["training"] = true,
                    ["contexts"] = 14,
                    ["auto_train"] = true
                },
                ["id"] = 0
            };

            client.TrySendFrame(waitTime, sessionRequest.ToString());

            string receivedMessage;
            client.TryReceiveFrameString(waitTime, out receivedMessage);
            Debug.Log(receivedMessage);
        }
        catch
        {
            CleanUp();
        }
    }

    private void Update()
    {
        Time.timeScale = timeScale;
        if (Time.time - lastActionTime > 0.1)
        {
            lastActionTime = Time.time;

            for (int i = 0; i < environments.Count(); i++)
            {
                var environment = environments[i];
                //var rotation = ((environment.botTransform.rotation.eulerAngles.z % 360) / 180) - 1;
                var rotation = (Mathf.PI / 180) * environment.botTransform.localRotation.eulerAngles.z;
                var rotationSin = Mathf.Sin(rotation);
                var rotationCos = Mathf.Cos(rotation);
                var xVelocity = environment.botRigidBody.velocity.x;
                var yVelocity = environment.botRigidBody.velocity.y;
                var aVelocity = environment.botRigidBody.angularVelocity / 100;
                var position = environment.botTransform.localPosition;
                if (aVelocity > 1)
                {
                    aVelocity = Mathf.Log10(aVelocity);
                }
                else if (aVelocity < -1)
                {
                    aVelocity = -Mathf.Log(Mathf.Abs(aVelocity));
                }

                var getActionRequest = new JObject
                {
                    ["jsonrpc"] = "2.0",
                    ["method"] = "get_action",
                    ["param"] = new JObject
                    {
                        ["inputs"] = new JArray { new JArray { rotationSin, rotationCos, xVelocity, yVelocity, aVelocity, position.x, position.y } },
                        ["context"] = i,
                        ["session_id"] = 0
                    },
                    ["id"] = 0
                };

                client.TrySendFrame(waitTime, getActionRequest.ToString());

                string receivedMessage;
                client.TryReceiveFrameString(waitTime, out receivedMessage);

                var actionMessage = JObject.Parse(receivedMessage);
                int action = (int)actionMessage["result"]["actions"][0];
                float value = (float)actionMessage["result"]["value"];
                environment.valueText.text = value.ToString("F2");

                switch (action)
                {
                    case 0:
                        environment.botController.movementController.MoveDirection = 1;
                        environment.botController.movementController.RotateDirection = 0;
                        break;
                    case 1:
                        environment.botController.movementController.MoveDirection = -1;
                        environment.botController.movementController.RotateDirection = 0;
                        break;
                    case 2:
                        environment.botController.movementController.MoveDirection = 0;
                        environment.botController.movementController.RotateDirection = 1;
                        break;
                    case 3:
                        environment.botController.movementController.MoveDirection = 0;
                        environment.botController.movementController.RotateDirection = -1;
                        break;
                    case 4:
                        environment.botController.movementController.MoveDirection = 0;
                        environment.botController.movementController.RotateDirection = 0;
                        break;
                }
                var reward = environment.reward;
                if (action == 1)
                {
                    reward -= 0.1f;
                }
                rewards.Add(reward);
                if (rewards.Count > 1000)
                {
                    rewards.RemoveAt(0);
                }
                rewardText.text = rewards.Average().ToString("F2");

                var giveRewardRequest = new JObject
                {
                    ["jsonrpc"] = "2.0",
                    ["method"] = "give_reward",
                    ["param"] = new JObject
                    {
                        ["reward"] = reward,
                        ["context"] = i,
                        ["session_id"] = 0
                    },
                    ["id"] = 0
                };

                client.TrySendFrame(waitTime, giveRewardRequest.ToString());
                client.TryReceiveFrameString(waitTime, out receivedMessage); 
                environment.reward = 0;
            }
        }
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
}
