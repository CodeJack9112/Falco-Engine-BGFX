using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class FirstPersonController : MonoBehaviour
{
	public int health = 100;
	public float jumpStrength = 10.0f;
	public float walkSpeed = 5.0f;
	public float runMultiplayer = 1.75f;
	public float gravity = -0.5f;
	public float maxFallSpeed = -25.0f;

	bool isGrounded = false;
	float runSpeed = 1.0f;

	float forwardSpeed = 0;
	float leftSpeed = 0;

	float fallSpeed = 0.0f;

	CapsuleCollider collider;

	bool locked = false;

	void Start()
	{
		Cursor.locked = true;
		Cursor.visible = false;

		collider = GetComponent<CapsuleCollider>();
	}

	void OnSceneLoaded()
    {
		
	}
	
	void Update()
	{
		if (Input.GetKey(ScanCode.LeftShift))
			runSpeed = runMultiplayer;
		else
			runSpeed = 1.0f;

		rigidbody.angularVelocity = new Vector3(0, 0, 0);

		forwardSpeed = 0;
		leftSpeed = 0;

		if (!locked)
		{
			if (Input.GetKey(ScanCode.W) || Input.GetKey(ScanCode.UpArrow))
				forwardSpeed = walkSpeed * runSpeed;

			if (Input.GetKey(ScanCode.S) || Input.GetKey(ScanCode.DownArrow))
				forwardSpeed = -walkSpeed * runSpeed;

			if (Input.GetKey(ScanCode.A) || Input.GetKey(ScanCode.LeftArrow))
				leftSpeed = walkSpeed * runSpeed;

			if (Input.GetKey(ScanCode.D) || Input.GetKey(ScanCode.RightArrow))
				leftSpeed = -walkSpeed * runSpeed;
		}

		rigidbody.linearVelocity = transform.forward * forwardSpeed + transform.left * leftSpeed + new Vector3(0, fallSpeed, 0);

		if (Input.GetKeyDown(ScanCode.Space))
		{
			if (isGrounded)
			{
				fallSpeed = jumpStrength;
			}
		}

		if (Input.GetKeyDown(ScanCode.L))
		{
			Cursor.locked = !Cursor.locked;
			Cursor.visible = !Cursor.locked;
		}

		if (Input.GetKey(ScanCode.LeftControl))
        {
			if (Input.GetKeyDown(ScanCode.R))
			{
				Time.timeScale = 1.0f;
				SceneManager.LoadScene(SceneManager.loadedScene);
			}
        }
	}

	void FixedUpdate()
	{
		if (!isGrounded)
		{
			if (fallSpeed > maxFallSpeed)
			{
				fallSpeed += gravity;
			}
		}

		float colH = collider.height * 0.5f + 0.1f;
		Rigidbody[] bodies = Physics.OverlapSphere(transform.position - new Vector3(0, colH, 0), collider.radius * 0.99f);

		int l = bodies.Length;
		
		foreach (Rigidbody b in bodies)
		{
			if (b != null)
			{
				if (b.gameObject == gameObject)
				{
					l -= 1;
				}
			}
		}

		if (isGrounded)
		{
			if (l == 0)
			{
				isGrounded = false;
			}
		}
		else
		{
			if (l > 0)
			{
				isGrounded = true;
				fallSpeed = 0.0f;
			}
		}
	}

    public void LockControls(bool value)
    {
		locked = value;
    }

	public bool IsMoving()
	{
		return forwardSpeed != 0 || leftSpeed != 0;
	}
}
