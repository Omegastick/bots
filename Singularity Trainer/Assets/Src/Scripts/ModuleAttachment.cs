using Scripts.Modules;
using UnityEngine;

namespace Scripts
{
    public class ModuleAttachment: MonoBehaviour
    {
        public Module parent;
        public Module child;

        private SpriteRenderer spriteRenderer;

        private void Awake()
        {
            spriteRenderer = GetComponent<SpriteRenderer>();
        }

        private void Update()
        {
            if (Application.isPlaying)
            {
                spriteRenderer.enabled = false;
            }
            else
            {
                spriteRenderer.enabled = true;
            }
        }
    }
}
