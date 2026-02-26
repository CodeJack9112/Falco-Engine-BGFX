using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FalcoEngine
{
    public struct AnimationClipInfo
    {
		/// <summary>
		/// Animation clip file
		/// </summary>
		public AnimationClip clip;

		/// <summary>
		/// Animation name
		/// </summary>
		public string name;

		/// <summary>
		/// Animation speed
		/// </summary>
		public float speed;

		/// <summary>
		/// Animation start frame
		/// </summary>
		public int startFrame;

		/// <summary>
		/// Animation end frame
		/// </summary>
		public int endFrame;

		/// <summary>
		/// Animation loop
		/// </summary>
		public bool loop;
	}
}
