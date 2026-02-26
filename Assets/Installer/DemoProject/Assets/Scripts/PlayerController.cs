using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class PlayerController : MonoBehaviour
{
    public float jumpStrength = 45.0f;
    public float walkSpeed = 25.0f;
    public float runMultiplayer = 2.0f;
    public float gravity = -25.0f;
    public GameObject weaponObject;
    public GameObject muzzleFlash;
    public GameObject camera;

    bool isGrounded = false;
    float runSpeed = 1.0f;
    float jumpSpeed = 0.0f;
    float jumpTimer = 0.0f;

    bool isShooting = false;
    float shootInterval = 10.0f;
    float currentShootInterval = 0.0f;
    float muzzleTimer = 5.0f;
    float currentMuzzleTimer = 0.0f;

    //Weapon bob
    public float MovementSpeed = 1;
    public float MovementAmount = 1;
    public float limitMin = -0.2f;
    public float limitMax = 0.2f;
    public float bobSpeed = 0.14f;

    float MovementX;
    float MovementY;
    Vector3 newGunPosition;
    Vector3 DefaultPosition;

    private float m_Time, m_Time2;
    float sinx, cosx = 0;

    float _speed;
    float pSpeed = 0;

    void Start()
	{
        currentShootInterval = shootInterval;

        if (muzzleFlash != null)
            muzzleFlash.visible = false;

        if (weaponObject != null)
            DefaultPosition = weaponObject.transform.localPosition;

        _speed = bobSpeed;
    }
		
	void Update()
	{
        Vector3 forward = rigidbody.rotation * new Vector3(0.0f, 0.0f, 1.0f);
        Vector3 left = rigidbody.rotation * new Vector3(1.0f, 0.0f, 0.0f);

        if (Input.GetKey(ScanCode.LeftShift))
        {
            runSpeed = runMultiplayer;
        }
        else
        {
            runSpeed = 1.0f;
        }

        rigidbody.angularVelocity = new Vector3(0, 0, 0);

        float forwardSpeed = 0;
        float leftSpeed = 0;

        if (Input.GetKey(ScanCode.W) || Input.GetKey(ScanCode.UpArrow))
        {
            forwardSpeed = -(walkSpeed * runSpeed);
        }

        if (Input.GetKey(ScanCode.S) || Input.GetKey(ScanCode.DownArrow))
        {
            forwardSpeed = walkSpeed * runSpeed;
        }

        if (Input.GetKey(ScanCode.A) || Input.GetKey(ScanCode.LeftArrow))
        {
            leftSpeed = -(walkSpeed * runSpeed);
        }

        if (Input.GetKey(ScanCode.D) || Input.GetKey(ScanCode.RightArrow))
        {
            leftSpeed = walkSpeed * runSpeed;
        }

        if (forwardSpeed != 0 || leftSpeed != 0)
        {
            m_Time += bobSpeed + pSpeed / 150;

            m_Time2 += _speed;

            if (sinx > 0.05f)
                _speed *= -1.0f;
            if (sinx < -0.05f)
                _speed *= -1.0f;

            sinx = Mathf.Lerp(sinx, Mathf.Sin(m_Time2) * 0.1f, 0.02f);
            cosx = Mathf.Lerp(cosx, Mathf.Cos(m_Time) * 0.1f, 0.02f);
        }
        else
        {
            sinx = Mathf.Lerp(sinx, 0, 0.01f);
            cosx = Mathf.Lerp(cosx, 0, 0.01f);
        }

        MovementX = Mathf.Lerp(MovementX, 0, 0.01f);
        MovementY = Mathf.Lerp(MovementY, 0, 0.01f);

        Vector2 mouse = Input.mouseMovement;
        MovementX += mouse.x * MovementAmount;
        MovementY += mouse.y * MovementAmount;

        if (MovementX > limitMax) MovementX = limitMax;
        if (MovementX < limitMin) MovementX = limitMin;
        if (MovementY > limitMax) MovementY = limitMax;
        if (MovementY < limitMin) MovementY = limitMin;

        newGunPosition = new Vector3(DefaultPosition.x, DefaultPosition.y, DefaultPosition.z);
        newGunPosition.x = Mathf.Lerp(DefaultPosition.x, DefaultPosition.x - (MovementX + sinx), MovementSpeed);
        newGunPosition.y = Mathf.Lerp(DefaultPosition.y, DefaultPosition.y + MovementY + cosx, MovementSpeed);

        if (weaponObject != null)
            weaponObject.transform.localPosition = Vector3.Lerp(weaponObject.transform.localPosition, newGunPosition, MovementSpeed);

        //First way to handle key down events
        if (Input.GetKeyDown(ScanCode.Space))
        {
            if (isGrounded)
            {
                jumpSpeed = jumpStrength;
                jumpTimer = 3.0f;
            }
        }

        rigidbody.linearVelocity = forward * forwardSpeed + left * leftSpeed + new Vector3(0, 1, 0) * jumpSpeed;
            
        //Detect collision with ground
        RaycastHit hit = Physics.Raycast(transform.position + new Vector3(0, -0.5f, 0), transform.position + new Vector3(0, -2.5f, 0));
        if (hit.hasHit)
        {
            if (hit.rigidbody != null)
            {
                isGrounded = true;
            }
            else
            {
                isGrounded = false;
            }
        }
        else
        {
            isGrounded = false;
        }

        if (!isGrounded)
        {
            if (jumpSpeed > 0)
            {
                jumpSpeed -= 0.25f;
            }
            else
            {
                jumpSpeed -= 0.3f;
            }
        }
        else
        {
            if (jumpTimer == 0.0f)
                jumpSpeed = 0;
        }

        if (jumpTimer > 0)
            jumpTimer -= 0.1f;
        else
            jumpTimer = 0.0f;

        //Fire
        if (weaponObject != null)
        {
            if (!weaponObject.animation.IsPlaying("Reload"))
            {
                if (isShooting)
                {
                    if (currentShootInterval >= shootInterval)
                    {
                        if (weaponObject != null && weaponObject.animation != null)
                        {
                            weaponObject.animation.Play("Fire");

                            if (muzzleFlash != null)
                            {
                                muzzleFlash.transform.rotation *= Quaternion.Euler(new Vector3(0, 0, 5));
                                muzzleFlash.visible = true;
                                currentMuzzleTimer = 0.0f;
                            }

                            if (audioSource != null)
                                audioSource.Play();

                            //Check shoot raycast
                            RaycastHit shootHit = Physics.Raycast(camera.transform.position, camera.transform.position + ((camera.transform.rotation * new Vector3(0.0f, 0.0f, -1.0f)) * 1000.0f));
                            if (shootHit.hasHit)
                            {
                                shootHit.rigidbody.AddForce(camera.transform.rotation * new Vector3(0.0f, 0.0f, -1.0f) * 500.0f, shootHit.hitPoint - shootHit.rigidbody.position);
                                DoDamage(shootHit.rigidbody.gameObject.transform, 25);
                                //MessageBox.Show(shootHit.rigidbody.gameObject.name);
                            }
                        }

                        currentShootInterval = 0.0f;
                    }
                    else
                    {
                        currentShootInterval += 0.1f;
                    }
                }
                else
                {
                    if (weaponObject != null && weaponObject.animation != null)
                    {
                        if (!weaponObject.animation.IsPlaying("Idle"))
                            weaponObject.animation.Play("Idle");
                    }
                }
            }
        }

        if (muzzleFlash != null)
        {
            if (muzzleFlash.visible)
            {
                if (currentMuzzleTimer < muzzleTimer)
                {
                    currentMuzzleTimer += 0.1f;
                }
                else
                {
                    muzzleFlash.visible = false;
                }
            }
        }
    }

    //Second way to handle key down events
    void KeyDown(ScanCode key)
    {
        if (key == ScanCode.R)
        {
            if (weaponObject != null && weaponObject.animation != null)
            {
                if (!weaponObject.animation.IsPlaying("Reload"))
                {
                    weaponObject.animation.Play("Reload");
                    weaponObject.audioSource.Play();
                }
            }
        }
    }

    void MouseDown(int button)
    {
        if (button == 0)
        {
            isShooting = true;
            currentShootInterval = shootInterval;
        }
    }

    void MouseUp(int button)
    {
        if (button == 0)
        {
            isShooting = false;
        }
    }

    void DoDamage(Transform from, int damage)
    {
        if (from.gameObject.GetMonoBehaviour("MonsterController") != null)
        {
            MonsterController cnt = (MonsterController)from.gameObject.GetMonoBehaviour("MonsterController");
            cnt.DoDamage(damage);
        }
        else
        {
            if (from.parent != null)
                DoDamage(from.parent, damage);
        }
    }
}