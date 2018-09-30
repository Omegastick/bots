using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Environment : MonoBehaviour {

    public GameObject bot;
    public GameObject background;
    public UnityEngine.UI.Text valueText;


    public Transform botTransform;
    public Rigidbody2D botRigidBody;
    public Bots.BotController botController;

    public float reward = 0;

	void Start () {
        botTransform = bot.GetComponent<Transform>();
        botRigidBody = bot.GetComponent<Rigidbody2D>();
        botController = bot.GetComponent<Bots.BotController>();
	}
	
	void Update () {
        if (botTransform.localPosition.x > 2)
        {
            reward += 100;
        }
        if (botTransform.localPosition.x < -2.5
            || botTransform.localPosition.x > 2.5
            || botTransform.localPosition.y < -2.5
            || botTransform.localPosition.y > 2.5)
        {
            Vector3 newPosition = new Vector3(0, 0, botTransform.localPosition.z);
            botTransform.localPosition = newPosition;
            botTransform.rotation = Quaternion.identity;
            botRigidBody.velocity = new Vector2(0, 0);
            botRigidBody.angularVelocity = 0;
            reward -= 10;
        }
    }
}
