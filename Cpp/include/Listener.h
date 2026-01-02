/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2020-2025 Lennart Andersson.
 *
 * This notice apply to all source files, *.cpp, *.h, *.java, and *.cs in this directory
 * and all its subdirectories if nothing else is explicitly stated within the source file itself.
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

#ifndef ops_ListenerH
#define ops_ListenerH


namespace ops
{
    //Forward declaration
    template<typename T> class Notifier;

    /// <summary>
    /// Interface that used in conjunction with Notifier forms an implementation of the
    /// Observer GoF-pattern.
    /// </summary>
    /// <typeparam name="ArgType"></typeparam>
    template<typename ArgType>
    class Listener
    {
    public:
        ///If this interface is registred with a Notifier, this method will be called when the
        ///Notifier wants to inform its Listeners that new data is available.
        virtual void onNewEvent(Notifier<ArgType>* sender, ArgType arg) = 0;
        virtual ~Listener() = default;
    };

    // Forward declaration
    template <typename T>
    class SingleNotifier;

    /// <summary>
    /// Interface that used in conjunction with SingleNotifier
    /// </summary>
    /// <typeparam name="ArgType"></typeparam>
    template <typename ArgType>
    class SingleListener
    {
      public:
        /// If this interface is registred with a SingleNotifier, this method will be called when the
        /// SingleNotifier wants to inform that new data is available.
        virtual void onNewEvent(SingleNotifier<ArgType>* sender, ArgType arg) = 0;
        virtual ~SingleListener() = default;
    };

} // namespace ops
#endif
