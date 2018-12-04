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
            List<bool> actions = Enumerable.Repeat(false, 9).ToList();
            if (Input.GetKey(KeyCode.Alpha1))
            {
                actions[0] = true;
            }
            if (Input.GetKey(KeyCode.Alpha2))
            {
                actions[1] = true;
            }
            if (Input.GetKey(KeyCode.Alpha3))
            {
                actions[2] = true;
            }
            if (Input.GetKey(KeyCode.Alpha4))
            {
                actions[3] = true;
            }
            if (Input.GetKey(KeyCode.Alpha5))
            {
                actions[4] = true;
            }
            if (Input.GetKey(KeyCode.Alpha6))
            {
                actions[5] = true;
            }
            if (Input.GetKey(KeyCode.Alpha7))
            {
                actions[6] = true;
            }
            if (Input.GetKey(KeyCode.Alpha8))
            {
                actions[7] = true;
            }
            if (Input.GetKey(KeyCode.Alpha9))
            {
                actions[8] = true;
            }
            agent.Act(actions);
        }
    }
}
