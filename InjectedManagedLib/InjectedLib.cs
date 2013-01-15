using UnityEngine;

namespace InjectedManagedLib
{
    public class InjectedScript : MonoBehaviour
    {
        void Start()
        {
            System.Diagnostics.Debug.WriteLine("=== I've been started ===");
        }

        void Update()
        {
            
        }

        void OnGUI()
        {
            if(GUI.Button(new Rect(10, 10, 150, 100), "I am a button"))
            {
                print("You clicked the button!");
            }
        }
    }

    public class InjectedLib
    {
        static void Initialize()
        {
            GameObject scriptContainer = new GameObject("InjectedScriptContainer");
            scriptContainer.AddComponent(typeof(InjectedScript));
        }
    }
}
