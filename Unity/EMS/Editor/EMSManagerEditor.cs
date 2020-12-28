
/**
 *  Haptics Framework
 *
 *  UHH HCI 
 *  Author: Oscar Ariza <ariza@informatik.uni-hamburg.de>
 *
 */

using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(EMSManager))]
public class EMSManagerEditor : Editor
{
    int signalDuration = 250;
    byte signalIntensity = 40;

    public override void OnInspectorGUI()
    {
        DrawDefaultInspector();

        EMSManager app = (EMSManager)target;

        GUILayout.Space(5);
        GUILayout.Label("EMS Signal Parameters", EditorStyles.boldLabel);
        signalDuration = (int)EditorGUILayout.Slider("Signal duration", signalDuration, 1, 1000);
        signalIntensity = (byte)EditorGUILayout.Slider("Signal intensity", signalIntensity, 1, 100);

        GUILayout.Space(15);

        if (GUILayout.Button("CH1 ON"))
        {
            app.CH1On();
        }
        else if (GUILayout.Button("CH1 OFF"))
        {
            app.CH1Off();
        }
        else if (GUILayout.Button("CH1 In/Decrease Intensity"))
        {
            app.CH1SetIntensity();
        }
        else if (GUILayout.Button("CH1 Trigger Peak"))
        {
            app.CH1SendPeakTriggerCommand();
        }

        GUILayout.Space(15);

        if (GUILayout.Button("CH2 ON"))
        {
            app.CH2On();
        }
        else if (GUILayout.Button("CH2 OFF"))
        {
            app.CH2Off();
        }
        else if (GUILayout.Button("CH2 In/Decrease Intensity"))
        {
            app.CH2SetIntensity();
        }
        else if (GUILayout.Button("CH2 Trigger Peak"))
        {
            app.CH2SendPeakTriggerCommand();
        }
    }
}
