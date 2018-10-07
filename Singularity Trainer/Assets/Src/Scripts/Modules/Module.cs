using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using SensorReadings;
using Actions;

namespace Scripts.Modules
{
    public abstract class Module : MonoBehaviour
    {
        public List<ModuleAttachment> ModuleAttachments { get; private set; }
        public List<Action> Actions { get; private set; }
        public Module parentModule;
        public virtual Module RootModule
        {
            get
            {
                return parentModule.RootModule;
            }
        }

        public virtual List<Module> GetChildren(List<Module> modules = null)
        {
            if (modules == null)
            {
                modules = new List<Module>();
            }
            modules.Add(this);
            foreach (var moduleAttachment in ModuleAttachments)
            {
                if (moduleAttachment.child != null)
                {
                    moduleAttachment.child.GetChildren(modules);
                }
            }
            return modules;
        }

        protected virtual void Awake()
        {
            Actions = new List<Action>();
            ModuleAttachments = GetComponentsInChildren<ModuleAttachment>().ToList();
        }

        public abstract ISensorReading GetSensorReading();
    }
}
