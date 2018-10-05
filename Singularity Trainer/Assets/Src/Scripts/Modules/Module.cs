using System;
using System.Collections.Generic;
using UnityEngine;
using SensorReadings;
using Scripts.Actions;

namespace Scripts.Modules
{
    public abstract class Module : MonoBehaviour
    {
        public List<ModuleAttachment> moduleAttachments;
        public List<Vector4> attachmentPoints;
        public List<BaseAction> actions;
        public virtual Module ParentModule { get; private set; }
        public virtual Module RootModule
        {
            get
            {
                return ParentModule.RootModule;
            }
        }

        public virtual List<Module> GetChildren(List<Module> modules = null)
        {
            if (modules == null)
            {
                modules = new List<Module>();
            }
            throw new NotImplementedException();
        }

        public abstract ISensorReading GetSensorReading();
    }
}
