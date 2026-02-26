using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows.Forms;

public class NavMeshAgentTest : MonoBehaviour
{
    public string objectName = "";
    GameObject target = null;

    void Start()
	{
        target = GameObject.Find(objectName);
        
	}
	
	void Update()
	{
        if (target != null)
            navMeshAgent.targetPosition = target.transform.position;
    }
}
