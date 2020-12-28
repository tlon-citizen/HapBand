
/**
 *  Haptics Framework
 *
 *  UHH HCI 
 *  Author: Oscar Ariza <ariza@informatik.uni-hamburg.de>
 *
 */

using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(EMSUnit))]
public class EMSUnitEditor : Editor
{
    public override void OnInspectorGUI()
    {
        DrawDefaultInspector();

        EMSUnit app = (EMSUnit)target;

        GUILayout.Space(15);

        if (GUILayout.Button("Toggle EMG Notification"))
        {
            app.ToggleEMGNotification();
        }
        else if (GUILayout.Button("Toggle Status Notification"))
        {
            app.ToggleStatusNotification();
        }
    }
}
