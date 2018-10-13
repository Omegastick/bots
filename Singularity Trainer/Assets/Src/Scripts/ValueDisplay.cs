using System.Collections.Generic;
using UnityEngine;

namespace Scripts
{
    public class ValueDisplay : MonoBehaviour
    {
        public int memoryLength = 100;
        private Renderer Renderer { get; set; }
        public Color goodColor;
        public Color badColor;

        private List<float> Values { get; set; }

        private void Awake()
        {
            Values = new List<float>();
            Renderer = GetComponent<Renderer>();
        }

        public void SetValue(float value)
        {
            Values.Add(value);
            if (Values.Count > memoryLength)
            {
                Values.RemoveAt(0);
            }
            UpdateColor();
        }

        private void UpdateColor()
        {
            float maxValue = Mathf.Max(Values.ToArray());
            float minValue = Mathf.Min(Values.ToArray());

            float latestValue = Values[Values.Count - 1];

            float normalizedValue = (latestValue - minValue) / (maxValue - minValue);

            Color valueColor = Color.Lerp(badColor, goodColor, normalizedValue);
            Renderer.material.SetColor("_Color", valueColor);
        }
    }
}
