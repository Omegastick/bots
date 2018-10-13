using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Scripts
{
    public class Chart : MonoBehaviour
    {
        public int maxSize = 1000;

        private LineRenderer LineRenderer { get; set; }
        private List<float> Data { get; set; }

        private void Awake()
        {
            LineRenderer = GetComponent<LineRenderer>();
            Data = new List<float>();
        }

        public void AddDataPoint(float dataPoint)
        {
            Data.Add(dataPoint);
        }

        public void Update()
        {
            List<float> selectedData;
            if (Data.Count > maxSize)
            {
                selectedData = new List<float>();
                float stepSize = Data.Count / maxSize;
                for (float i = 0; i < Data.Count; i += stepSize)
                {
                    selectedData.Add(Data[(int)i]);
                }
            }
            else
            {
                selectedData = Data;
            }

            var minValue = Data.Min();
            var maxValue = Data.Max();
            var range = maxValue - minValue;
            if (range == 0)
            {
                range = 1f;
            }

            var normalisedData = selectedData.Select(d => (d - minValue) / range).ToList();

            List<Vector3> normalisedPoints = new List<Vector3>();
            for (int i = 0; i < selectedData.Count; i++)
            {
                Vector3 point = new Vector3
                {
                    y = normalisedData[i],
                    x = (float)i / selectedData.Count
                };
                normalisedPoints.Add(point);
            }
            LineRenderer.positionCount = selectedData.Count;
            LineRenderer.SetPositions(normalisedPoints.ToArray());
        }
    }
}
