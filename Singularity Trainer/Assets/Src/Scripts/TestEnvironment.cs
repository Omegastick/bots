using System.Collections.Generic;
using UnityEngine;

namespace Scripts
{
    public class TestEnvironment : MonoBehaviour
    {
        public Agent agent;

        private void Update()
        {
            if (Input.GetKeyDown(KeyCode.Space))
            {
                Debug.Log("Sending action");
                agent.Act(new List<int> { 1 });
            }
            else
            {
                agent.Act(new List<int> { 0 });
            }
        }
    }
}
