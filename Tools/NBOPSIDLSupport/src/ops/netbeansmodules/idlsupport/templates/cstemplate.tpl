//Auto generated OPS-code. DO NOT MODIFY!

using Ops;
using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace __packageName
{
    // Message

__classComment
    [Serializable]
    public class __className : __baseClassName
    {
        private byte ___className_version = __className_idlVersion;
        public byte __className_version
        {
            get { return ___className_version; }
            set {
                if (value > __className_idlVersion)
                {
                    throw new IdlVersionException(
                        "__className: received version '" + value + "' > known version '" + __className_idlVersion + "'");
                }
                ___className_version = value;
            }
        }

__declarations

        public static new string GetTypeName() { return "__packageName.__className"; }

        public __className() : base()
        {
            AppendType(GetTypeName());
__constructorBody
        }

        public override void Serialize(IArchiverInOut archive)
        {
            base.Serialize(archive);
__serialize
        }

        public override object Clone()
        {
            __className cloneResult = new __className();
            FillClone(cloneResult);
            return cloneResult;
        }

        public override void FillClone(OPSObject cloneO)
        {
            base.FillClone(cloneO);
            __className cloneResult = (__className)cloneO;
__cloneBody
        }
    }

    // Publisher

    public class __classNamePublisher : Publisher
    {
        public __classNamePublisher(Topic t) : base(t)
        {
            CheckTypeString(__className.GetTypeName());
        }

        public void Write(__className o)
        {
            base.Write(o);
        }
    }

    // Subscriber

    public delegate void __classNameEventHandler(__classNameSubscriber sender, __className data);

    public class __classNameSubscriber : Subscriber
    {
        public event __classNameEventHandler newData;

        public __classNameSubscriber(Topic t) : base(t)
        {
            CheckTypeString(__className.GetTypeName());
        }

        protected override void NewDataArrived(OPSObject o)
        {
            if (newData != null) newData(this, (__className)o);
        }

        public __className WaitForNextData(long millis)
        {
            return (__className)WaitForNextOpsObjectData(millis);
        }
    }

}
