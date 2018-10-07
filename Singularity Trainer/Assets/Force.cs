using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Force : MonoBehaviour {

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        Debug.Log(transform.TransformDirection(new Vector2(0, 1)));
        GetComponent<Rigidbody2D>().AddForceAtPosition(transform.TransformDirection(new Vector2(0, 1)), transform.TransformPoint(new Vector2(1, 0)));
	}
}
