using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Scripts
{
    public class TestEnvironment : MonoBehaviour
    {
        public Agent agent;

        private void Update()
        {
            List<int> actions = Enumerable.Repeat(0, 9).ToList();
            if (Input.GetKey(KeyCode.Alpha1))
            {
                actions[0] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha2))
            {
                actions[1] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha3))
            {
                actions[2] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha4))
            {
                actions[3] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha5))
            {
                actions[4] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha6))
            {
                actions[5] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha7))
            {
                actions[6] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha8))
            {
                actions[7] = 1;
            }
            if (Input.GetKey(KeyCode.Alpha9))
            {
                actions[8] = 1;
            }
            agent.Act(actions);
        }
    }
}
