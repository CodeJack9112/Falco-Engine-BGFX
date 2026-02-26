using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class PlayVideo : MonoBehaviour
{
	public Material material;

	void Start()
	{
		VideoPlayer videoPlayer = GetComponent<VideoPlayer>();

		//Bind output texture to material
		material.SetParameter("albedoMap", videoPlayer.texture);

		//Listen when video player reach the last frame
        videoPlayer.onEnded += Player_onEnded;
	}

    private void Player_onEnded(VideoPlayer sender)
    {
        if (sender.loop)
        {
			AudioSource audioSource = GetComponent<AudioSource>();
			if (audioSource != null)
			{
				//Force restart an audio source is needed to synchronize a video with an audio
				audioSource.Play();
			}
        }
    }

    void Update()
	{
		
	}
}
