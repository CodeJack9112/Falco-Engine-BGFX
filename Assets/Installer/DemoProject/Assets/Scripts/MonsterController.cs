using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows.Forms;

public class MonsterController : MonoBehaviour
{
    public int Health = 100;

    bool died = false;

    void Start()
    {
        ActivateRagdoll(transform, false);
    }

    void Update()
    {

    }

    void ActivateRagdoll(Transform root, bool active)
    {
        if (root.gameObject.rigidbody != null)
        {
            root.gameObject.rigidbody.isKinematic = !active;
        }

        for (int i = 0; i < root.childCount; i++)
        {
            if (root.GetChild(i).gameObject.rigidbody != null)
                root.GetChild(i).gameObject.rigidbody.isKinematic = !active;

            ActivateRagdoll(root.GetChild(i), active);
        }
    }

    public void DoDamage(int damage)
    {
        //MessageBox.Show(damage.ToString());

        if (died)
            return;

        Health -= damage;

        if (Health <= 0)
        {
            Die();
        }
    }

    public void Die()
    {
        Health = 0;
        died = true;

        animation.Stop();

        if (navMeshAgent != null)
            navMeshAgent.enabled = false;

        ActivateRagdoll(transform, true);
    }
}
