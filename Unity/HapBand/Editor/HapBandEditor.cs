
/**
 *  Haptics Framework
 *
 *  UHH HCI 
 *  Author: Oscar Ariza <ariza@informatik.uni-hamburg.de>
 *
 */

using UnityEngine;
using UnityEditor;
using GraphicDNA;

[CustomEditor(typeof(HapBandManager))]
public class HapBandEditor : Editor
{
    private Rect targetRect;
    private HapBandManager manager = null;

    [HideInInspector] public bool enabledA = true;
    [HideInInspector] public bool enabledB = true;
    [HideInInspector] public bool waitA = true;
    [HideInInspector] public bool waitB = true; 
    [HideInInspector] public byte volumeA = 100; // 0 .. 100
    [HideInInspector] public byte volumeB = 100; // 0 .. 100

    private bool lastWaitA = true;
    private bool lastWaitB = true;
    private byte lastVolumeA = 100; // 0 .. 100
    private byte lastVolumeB = 100; // 0 .. 100
    private LRAMode lastLRAMode = LRAMode.EffectMode;  

    protected Color ToColor(int r, int g, int b, int a = 255)
    {
        return new Color((float)r / 255f, (float)g / 255f, (float)b / 255f, (float)a / 255f);
    }
    
    public override void OnInspectorGUI()
    {
        DrawDefaultInspector();

        if (target != null)
            manager = (HapBandManager)target;
        else
            GUILayout.Label("Device not connected yet!");

        GUILayout.Space(5);

        if (manager.isReady()) // Application.isPlaying
        {
            GUILayout.BeginHorizontal();
            GUILayout.Label("Device Name", EditorStyles.boldLabel);
            if (target != null) manager.device.deviceName = GUILayout.TextField(manager.device.deviceName, EditorStyles.boldLabel);
            GUILayout.EndHorizontal();

            GUILayout.Space(5);
            GUILayout.Label("Sensor Notifications", EditorStyles.boldLabel);  
            EditorGUI.BeginDisabledGroup(true);
            if (target != null && manager.device != null) EditorGUILayout.Toggle("Enabled", manager.device.notifications);
            EditorGUI.EndDisabledGroup();
            if (GUILayout.Button("Toggle notifications"))
            {
                manager.toggleNotifications();
            }
            GUILayout.BeginHorizontal();
            GUILayout.Label("Pressure Sensor");
            if (target != null) GUILayout.Label(manager.device.pressureSensor.ToString());
            GUILayout.EndHorizontal();
            GUILayout.BeginHorizontal();
            GUILayout.Label("Capacitive Sensors");
            if (target != null && manager.device != null) GUILayout.Label(manager.device.PrintCapacitiveStatus());
            GUILayout.EndHorizontal();
        }

        ///////////////////

        targetRect = GUILayoutUtility.GetRect((EditorGUIUtility.currentViewWidth * 0.9f), 300); /// https://gamedev.stackexchange.com/questions/141302/how-do-i-draw-lines-to-a-custom-inspector
        if (Event.current.type == EventType.Repaint)
        {
            float x = 10;
            float y = 10 + targetRect.y;

            GUI.DrawTexture(new Rect(x, y, 300, 260), manager.HandTexture);
            GUI.Label(new Rect(x + 150, y + 100, 100, 50), "Solenoid A");
        }

        ///////////////////

        GUILayout.Label("LRA Actuator", EditorStyles.boldLabel);
        manager.lraIndex = (byte)EditorGUILayout.Slider("LRA IDX", manager.lraIndex, 0, 5);
        manager.effectID = (byte)EditorGUILayout.Slider("Effect IDX", manager.effectID, 1, 123); // Page 57 on http://www.ti.com/lit/ds/symlink/drv2605.pdf 
        manager.realTimeValue = (byte)EditorGUILayout.Slider("Real Time Value", manager.realTimeValue, 0, 127);
        manager.lraMode = (LRAMode)EditorGUILayout.EnumPopup("Mode", manager.lraMode);
        if (lastLRAMode != manager.lraMode) manager.setLRAMode(manager.lraMode);
        lastLRAMode = manager.lraMode;
        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Trigger LRA")) manager.triggerLRA(true);
        if (GUILayout.Button("Stop LRA")) manager.triggerLRA(false);
        GUILayout.EndHorizontal();

        ///////////////////

        GUILayout.Space(5);
        GUILayout.Label("Piezo Actuator", EditorStyles.boldLabel);
        manager.piezoIndex = (byte)EditorGUILayout.Slider("Piezo IDX", manager.piezoIndex, 0, 5);
        manager.frequency = (int)EditorGUILayout.Slider("Frequency", manager.frequency, 8, 1992); // Frequency 7.8125 Hz -- 1992,1875 Hz
        manager.amplitud = (byte)EditorGUILayout.Slider("Amplitud", manager.amplitud, 0, 255);
        manager.cycles = (byte)EditorGUILayout.Slider("Cycles", manager.cycles, 0, 255);
        manager.envelope = (byte)EditorGUILayout.Slider("Envelope IDX", manager.envelope, 0, 50);
        manager.durationMS = 1000.0f * ((float)manager.cycles / (float)manager.frequency); // TODO: add the envelope time. Page 20 on http://www.ti.com/lit/ds/symlink/drv2667.pdf
        GUILayout.BeginHorizontal();
        GUILayout.Label("Duration (MS)");
        GUILayout.Label(manager.durationMS.ToString());
        GUILayout.EndHorizontal();
        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Trigger Piezo")) manager.triggerPiezo(true);
        if (GUILayout.Button("Stop Piezo")) manager.triggerPiezo(false);
        GUILayout.EndHorizontal();

        ///////////////////

        GUILayout.Space(5);
        GUILayout.Label("Haptic Reactors", EditorStyles.boldLabel);
        manager.audioIndex = (byte)EditorGUILayout.Slider("Audio IDX", manager.audioIndex, 0, 10);

        enabledA = (bool)EditorGUILayout.Toggle("Enabled A", enabledA);
        waitA = (bool)EditorGUILayout.Toggle("Wait before play A", waitA);
        if (!waitA.Equals(lastWaitA)) manager.SetWaitFlagA(waitA);
        lastWaitA = waitA;
        volumeA = (byte)EditorGUILayout.Slider("Volume A", volumeA, 0, 100);
        if (!volumeA.Equals(lastVolumeA)) manager.SetVolumeA(volumeA);
        lastVolumeA = volumeA;

        enabledB = (bool)EditorGUILayout.Toggle("Enabled B", enabledB);
        waitB = (bool)EditorGUILayout.Toggle("Wait before play B", waitB);
        if (!waitB.Equals(lastWaitB)) manager.SetWaitFlagB(waitB);
        lastWaitB = waitB;
        volumeB = (byte)EditorGUILayout.Slider("Volume B", volumeB, 0, 100);
        if (!volumeB.Equals(lastVolumeB)) manager.SetVolumeB(volumeB);
        lastVolumeB = volumeB;
        GUILayout.BeginHorizontal();
            if (GUILayout.Button("Trigger Haptic Reactor")) manager.triggerAudio(true, enabledA, enabledB);
            if (GUILayout.Button("Stop Haptic Reactor")) manager.triggerAudio(false, enabledA, enabledB);
        GUILayout.EndHorizontal();
        if (GUILayout.Button("Stream Audio Data"))
        {
            manager.PlayStream(enabledA, enabledB); 
        }

        ///////////////////

        GUILayout.Space(5);
        GUILayout.Label("Solenoids", EditorStyles.boldLabel);
        manager.solenoidIndex = (byte)EditorGUILayout.Slider("Solenoid IDX", manager.solenoidIndex, 1, 2);
        manager.feedbackOutput = (FeedbackOutput)EditorGUILayout.EnumPopup("Output", manager.feedbackOutput);
        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Trigger Feedback"))
        {
            manager.triggerFeedback(true, enabledA, enabledB);
        }
        if (GUILayout.Button("Stop Feedback"))
        {
            manager.triggerFeedback(false, enabledA, enabledB);
        }
        GUILayout.EndHorizontal();

        ///GUILayout.BeginHorizontal();
        ///GUILayout.Label("Battery Level"); GUILayout.Label(manager.batteryLevel);
        ///GUILayout.EndHorizontal();

        ///////////////////////////////////////////////////////////////////////////////////////////////////

        if (EditorApplication.isPlaying)
            Repaint();
    }
}
