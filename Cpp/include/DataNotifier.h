/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2018-2020 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <vector>
#include <functional>

#include "OPSTypeDefs.h"
#include "OPSExport.h"
#include "DataListener.h"

namespace ops
{
    ///Class which in the conjunction with DataListener forms an implementation of the
    ///observer GoF-pattern. Classes extending this class extends an interface to which
    ///DataListeners can register their interest to be notified when new OPSObjects are available.
	///There is also a possibility to use a callback instead of the DataListener interface.
    class OPS_EXPORT DataNotifier
    {
    public:
        typedef void (*CallbackFunc)(DataNotifier* sender, void* userData);

        ///Register a DataListener that uses callbacks
#ifdef OPS_C14_DETECTED
        [[deprecated("Deprecated. Replaced by the more flexible addDataListener(std::function<...>) interface")]]
#endif
        void addDataListener(CallbackFunc func, void* userData)
        {
            addDataListener([=](DataNotifier* sender) { func(sender, userData); });
        }
        
        ///Register a DataListener
        void addDataListener(DataListener* listener)
        {
            addDataListener([=](DataNotifier* sender) { listener->onNewData(sender); });
        }
        
        ///Register a DataListener that uses a closure
        void addDataListener(std::function<void(DataNotifier* sender)> callback);

        //Destructor:
        virtual ~DataNotifier();

    protected:
        ///Vector that holds pointers to DataListeners using closures
        std::vector<std::function<void(DataNotifier* sender)>> closureListeners;

        ///Called by subclasses that wishes to notify its listeners of the arrival of new data.
        void notifyNewData();
    };

}
