using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Scripts
{
    public class Chart : MonoBehaviour
    {
        public int maxSize = 100;
        public List<float> SmoothedData { get; private set; }

        private LineRenderer LineRenderer { get; set; }
        private List<float> Data { get; set; }

        private void Awake()
        {
            LineRenderer = GetComponent<LineRenderer>();
            Data = new List<float>();
            SmoothedData = new List<float>();
        }

        public void AddDataPoint(float dataPoint)
        {
            Data.Add(dataPoint);
        }

        public void Update()
        {
            if (Data.Count < 2)
            {
                return;
            }
            SmoothedData = Smooth(Data, 0.99f);
            List<float> selectedData;
            if (SmoothedData.Count > maxSize)
            {
                selectedData = new List<float>();
                float stepSize = SmoothedData.Count / maxSize;
                for (float i = 0; i < SmoothedData.Count; i += stepSize)
                {
                    selectedData.Add(SmoothedData[(int)i]);
                }
            }
            else
            {
                selectedData = SmoothedData;
            }

            var minValue = selectedData.Min();
            var maxValue = selectedData.Max();
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

        private static List<float> Smooth(List<float> data, float smoothingWeight)
        {
            var smoothedData = new List<float>();
            float runningAverage = data[0];
            foreach (float dataPoint in data)
            {
                runningAverage = runningAverage * smoothingWeight + (1 - smoothingWeight) * dataPoint;
                smoothedData.Add(runningAverage);
            }
            return smoothedData;
        }
    }
}
