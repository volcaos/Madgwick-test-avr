using System.Collections.Generic;
using System.IO.Ports;
using System.Threading;
using UnityEngine;
using System.Linq;
using UnityEngine.UI;
using System.Text;
using System;

public class quot_read : MonoBehaviour
{
	public Text text;
	public List<float[]> dofData;
	// 
	SerialPort serialPort;

	StringBuilder stringBuilder = new StringBuilder();
	bool hasData = false;
	string lineData;
	// 
	Thread thread;

    // 
    void Start()
    {
		resetDofData();
		// 
        serialPort = new SerialPort("COM8",1000000,Parity.None,8,StopBits.One);
		serialPort.NewLine = "\n";
		serialPort.Open();
		// 
		thread = new Thread(comRead);
		thread.Start();
    }

	private void comRead(object obj)
	{
		while( serialPort != null && serialPort.IsOpen ){
			var d = serialPort.ReadExisting();
			stringBuilder.Append(d);
		}
	}

	private void SerialPort_DataReceived( object sender, SerialDataReceivedEventArgs e )
	{
		stringBuilder.Append( serialPort.ReadExisting() );
		string s = stringBuilder.ToString();
		if( s.Contains('\n') ){
			var ss = s.Split('\n');
			lineData = ss[0];
			stringBuilder = new StringBuilder(ss[1]);
			hasData = true;
		}
	}
	// 
	void Update()
	{
		if( hasData ){
			float[] q = lineData.Trim().Split(',').Select( s => {
				float f;
				float.TryParse(s,out f);
				return f;
			} ).ToArray();
			// 
			if( q.Length == 4 ){
				Quaternion quat = new Quaternion(q[0],q[1],q[2],q[3]);
				transform.rotation = Quaternion.identity;
				transform.Rotate(quat.eulerAngles,Space.Self);
				//transform.localRotation = quat.normalized;
				// 
				text.text = string.Format("{0:+0.00},{1:+0.00},{2:+0.00},{3:+0.00}",q[0],q[1],q[2],q[3]);
			}else if( q.Length == 9 ){
				dofData.Add( q );
				//
				if( dofData.Count >= 2 ){
					for( int i=0; i<dofData.Count-1; i++ ){
						for( int j=0; j<9; j++ ){
							Debug.DrawLine(
								new Vector3(i,dofData[i][j]),
								new Vector3(i,dofData[i+1][j]) );
						}
					}
				}
				text.text = dofData.Count.ToString();
			}else{
				Debug.Log(lineData);
			}
			hasData = false;
		}
	}
	//
	void resetDofData()
	{
		dofData = new List<float[]>();
	}
}
