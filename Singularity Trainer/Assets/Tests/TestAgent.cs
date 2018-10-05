using UnityEngine;
using UnityEngine.TestTools;
using NUnit.Framework;
using System.Collections;
using Scripts;

public class TestAgent {

    [Test]
    public void GetObservationReturns() {
        var go = new GameObject("Agent");
        var agent = go.AddComponent<Agent>();
        var observation = agent.GetObservation();
        Assert.NotNull(observation);
    }

    // A UnityTest behaves like a coroutine in PlayMode
    // and allows you to yield null to skip a frame in EditMode
    [UnityTest]
    public IEnumerator TestAgentWithEnumeratorPasses() {
        // Use the Assert class to test conditions.
        // yield to skip a frame
        yield return null;
    }
}
