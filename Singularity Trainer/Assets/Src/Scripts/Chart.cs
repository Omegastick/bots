using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Scripts
{
    public class Chart : MonoBehaviour
    {
        public int maxSize = 100;
        public float smoothingWeight;
        public List<float> SmoothedData { get; private set; }

        private LineRenderer LineRenderer { get; set; }
        private List<float> NewData { get; set; }
        private float MaxPoint { get; set; }
        private float MinPoint { get; set; }

        private void Awake()
        {
            LineRenderer = GetComponent<LineRenderer>();
            NewData = new List<float>();
            SmoothedData = new List<float>();
            MaxPoint = Mathf.NegativeInfinity;
            MinPoint = Mathf.Infinity;
        }

        public void AddDataPoint(float dataPoint)
        {
            NewData.Add(dataPoint);
        }

        public void Update()
        {
            if (SmoothedData.Count < 2 && NewData.Count < 2)
            {
                return;
            }
            SmoothNewData();
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

            var range = MaxPoint - MinPoint;
            if (range == 0)
            {
                range = 1f;
            }

            var normalisedData = selectedData.Select(d => (d - MinPoint) / range).ToList();

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
            LineRenderer.positionCount = normalisedPoints.Count;
            LineRenderer.SetPositions(normalisedPoints.ToArray());
        }

        private void SmoothNewData()
        {
            if (NewData.Count < 2)
            {
                return;
            }
            float runningAverage;
            if (SmoothedData.Count > 1)
            {
                runningAverage = SmoothedData.Last();
            }
            else
            {
                runningAverage = NewData[0];
            }
            foreach (float dataPoint in NewData)
            {
                runningAverage = runningAverage * smoothingWeight + (1 - smoothingWeight) * dataPoint;
                SmoothedData.Add(runningAverage);
                MaxPoint = Mathf.Max(MaxPoint, runningAverage);
                MinPoint = Mathf.Min(MinPoint, runningAverage);
            }
            NewData.Clear();
        }
    }
}
