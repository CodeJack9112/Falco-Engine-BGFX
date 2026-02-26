using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows.Forms;

public class CameraController : MonoBehaviour
{
    public GameObject root;
    public float sensivity = 0.12f;

    float rotationX = 0F;
    float rotationY = 0F;

    Quaternion originalRotation;
    Quaternion originalRotationRoot;

    void Start()
	{
        originalRotation = transform.rotation;
        originalRotationRoot = root.rigidbody.rotation;

        if (root != null)
        {
            //MessageBox.Show(root.name);
        }

    }
		
	void Update()
	{
        Vector2 mouse = Input.mouseMovement;
        rotationX -= mouse.y * sensivity;
        rotationY -= mouse.x * sensivity;

        rotationX = ClampAngle(rotationX, -80.0f, 80.0f);
        rotationY = ClampAngle(rotationY, -360.0f, 360.0f);

        Quaternion yQuaternion = Quaternion.AngleAxis(rotationX, new Vector3(1, 0, 0));
        Quaternion xQuaternion = Quaternion.AngleAxis(rotationY, new Vector3(0, 1, 0));

        transform.rotation = originalRotation * xQuaternion * yQuaternion;
        root.rigidbody.rotation = originalRotationRoot * xQuaternion;
    }

    public float ClampAngle(float angle, float min, float max)
    {
        angle = angle % 360;
        if ((angle >= -360F) && (angle <= 360F))
        {
            if (angle < -360F)
            {
                angle += 360F;
            }
            if (angle > 360F)
            {
                angle -= 360F;
            }
        }

        return Mathf.Clamp(angle, min, max);
    }
}