using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class MouseLook : MonoBehaviour
{
	public float sensivity = 1.0f;
	public bool lookX = true;
	public bool lookY = true;

	public bool useRightMouseButton = false;

	public bool escapeForQuit = false;

	float rotationX = 0.0f;
	float rotationY = 0.0f;

	Quaternion originalRotation = Quaternion.identity;

	void Start()
	{
		originalRotation = transform.localRotation;

		if (!useRightMouseButton)
		{
			Cursor.locked = true;
			Cursor.visible = false;
		}
	}
	
	void Update()
	{
		bool update = true;
		if (useRightMouseButton)
        {
			update = Input.GetMouseButton(1);
			Cursor.locked = update;
			Cursor.visible = !update;
		}

		if (update)
		{
			Vector2 mouse = Input.cursorDirection;

			if (lookX)
				rotationX += mouse.y * sensivity * 0.1f;

			if (lookY)
				rotationY -= mouse.x * sensivity * 0.1f;

			rotationX = ClampAngle(rotationX, -80.0f, 80.0f);
			rotationY = ClampAngle(rotationY, -360.0f, 360.0f);

			Quaternion yQuaternion = Quaternion.AngleAxis(rotationX, Vector3.right);
			Quaternion xQuaternion = Quaternion.AngleAxis(rotationY, Vector3.up);

			transform.localRotation = originalRotation * xQuaternion * yQuaternion;
		}

		if (!useRightMouseButton)
		{
			if (Input.GetKeyDown(ScanCode.L))
			{
				Cursor.locked = !Cursor.locked;
				Cursor.visible = !Cursor.locked;
			}

			if (Input.GetKeyDown(ScanCode.Escape))
			{
				Cursor.locked = false;
				Cursor.visible = true;
			}
		}

		if (escapeForQuit)
        {
			if (Input.GetKeyDown(ScanCode.Escape))
				Application.Quit();
        }
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
