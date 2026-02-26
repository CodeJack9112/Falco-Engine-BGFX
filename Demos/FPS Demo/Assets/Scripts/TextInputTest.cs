using FalcoEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class TextInputTest : MonoBehaviour
{
	public TextInput textInput;
	public Text text;

	void Start()
	{
		
	}
	
	void Update()
	{
		text.text = textInput.text;

		if (Input.GetKeyDown(ScanCode.Return))
        {
			textInput.text = textInput.text.ToUpper();
        }
	}
}
