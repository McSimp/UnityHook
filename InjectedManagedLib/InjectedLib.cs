using UnityEngine;

namespace InjectedManagedLib
{
    public class InjectedLib
    {
        static bool goingUp = false;
        static int numCalls = 0;

        static void DoTheThing()
        {
            numCalls++;
            if (numCalls < 200)
            {
                return;
            }

            GameObject player = GameObject.FindGameObjectWithTag("Player");
            if (player.Equals(null))
            {
                System.Diagnostics.Debug.WriteLine("Couldn't find player!");
                return;
            }

            Vector3 scalar = new Vector3(0.1f, 0.1f, 0.1f);
            if (goingUp)
            {
                player.transform.localScale += scalar;
            }
            else
            {
                player.transform.localScale -= scalar;
            }

            if (player.transform.localScale.x >= 2.0f)
            {
                goingUp = false;
            }
            else if (player.transform.localScale.x <= 0.2f)
            {
                goingUp = true;
            }
        }
    }
}
