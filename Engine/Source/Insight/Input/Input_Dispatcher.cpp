#include "Engine_pch.h"

#include "Input_Dispatcher.h"


namespace Insight {

	namespace Input {

		InputDispatcher* InputDispatcher::s_Instance = nullptr;

		void InputDispatcher::UpdateInputs()
		{
			for (uint32_t i = 0; i < m_AxisMappings.size(); i++)
			{
				SHORT KeyState = ::GetAsyncKeyState(m_AxisMappings[i].MappedKeyCode);
				bool Pressed = (BIT_SHIFT(15)) & KeyState;
				if (Pressed)
				{
					KeyPressedEvent e(m_AxisMappings[i].MappedKeyCode, 0);
					ProcessInputEvent(e);
				}
			}
		}

		void InputDispatcher::ProcessInputEvent(Event& e)
		{
			EventDispatcher Dispatcher(e);
			// Mouse Buttons
			Dispatcher.Dispatch<MouseMovedEvent>(IE_BIND_EVENT_FN(InputDispatcher::DispatchMouseMoveEvent));
			Dispatcher.Dispatch<MouseButtonPressedEvent>(IE_BIND_EVENT_FN(InputDispatcher::DispatchActionEvent));
			Dispatcher.Dispatch<MouseButtonReleasedEvent>(IE_BIND_EVENT_FN(InputDispatcher::DispatchActionEvent));
			// Mouse Scroll
			Dispatcher.Dispatch<MouseScrolledEvent>(IE_BIND_EVENT_FN(InputDispatcher::DispatchMouseScrolledEvent));

			// Key Pressed
			Dispatcher.Dispatch<KeyPressedEvent>(IE_BIND_EVENT_FN(InputDispatcher::DispatchKeyPressEvent));
			Dispatcher.Dispatch<KeyPressedEvent>(IE_BIND_EVENT_FN(InputDispatcher::DispatchActionEvent));
			Dispatcher.Dispatch<KeyReleasedEvent>(IE_BIND_EVENT_FN(InputDispatcher::DispatchActionEvent));

			// Key Typed
			//Dispatcher.Dispatch<KeyTypedEvent>(IE_BIND_EVENT_FN(InputManager::OnKeyTypedEvent));
		}

		void InputDispatcher::RegisterAxisCallback(const char* Name, EventInputAxisFn Callback)
		{
			m_AxisCallbacks[Name].push_back(Callback);
		}

		void InputDispatcher::RegisterActionCallback(const char* Name, InputEventType EventType, EventInputActionFn Callback)
		{
			m_ActionCallbacks[{Name, EventType}].push_back(Callback);
		}

		bool InputDispatcher::DispatchKeyPressEvent(KeyPressedEvent& e)
		{
			for (AxisMapping& Axis : m_AxisMappings)
			{
				// Find the key in the axis map.
				if (Axis.MappedKeyCode == e.GetKeyCode())
				{
					// Use the keycode as the key into the axis map to find 
					// the callbacks associated with it.
					auto Callbacks = &m_AxisCallbacks[Axis.Hint];
					for (EventInputAxisFn Callback : *Callbacks)
					{
						// Call the callbacks.
						Callback(Axis.Scale);
					}
				}
			}
			return false;
		}

		bool InputDispatcher::DispatchMouseScrolledEvent(MouseScrolledEvent& e)
		{
			for (AxisMapping& Axis : m_AxisMappings)
			{
				// Find the key in the axis map.
				if (Axis.MappedKeyCode == e.GetKeyCode())
				{
					float MoveFactor = e.GetYOffset();

					// Use the keycode as the key into the axis map to find 
					// the callbacks associated with it.
					auto Callbacks = &m_AxisCallbacks[Axis.Hint];
					for (EventInputAxisFn Callback : *Callbacks)
					{
						// Invoke the callbacks.
						Callback(MoveFactor);
					}
				}
			}
			return false;
		}

		bool InputDispatcher::DispatchMouseMoveEvent(MouseMovedEvent& e)
		{
			for (AxisMapping& Axis : m_AxisMappings)
			{
				// Find the key in the axis map.
				if (Axis.MappedKeyCode == e.GetKeyCode())
				{
					float MoveFactor = 0.0f;
					if (e.GetKeyCode() == KeyMapCode_Mouse_MoveX) MoveFactor = e.GetX();
					if (e.GetKeyCode() == KeyMapCode_Mouse_MoveY) MoveFactor = e.GetY();

					// Use the keycode as the key into the axis map to find 
					// the callbacks associated with it.
					auto Callbacks = &m_AxisCallbacks[Axis.Hint];
					for (EventInputAxisFn Callback : *Callbacks)
					{
						// Invoke the callbacks.
						Callback(MoveFactor);
					}
				}
			}
			return false;
		}

		bool InputDispatcher::DispatchActionEvent(InputEvent& e)
		{
			for (ActionMapping& Action : m_ActionMappings)
			{
				if (Action.MappedKeycode == e.GetKeyCode())
				{
					if (e.GetEventType() == InputEventType_Released)
					{
						Action.CanDispatch = true;
					}

					if (Action.CanDispatch)
					{
						auto Callbacks = &m_ActionCallbacks[{Action.Hint, e.GetEventType()}];
						for (EventInputActionFn Callback : *Callbacks)
						{
							// Invoke the callbacks.
							Callback();
						}
					}

					if (e.GetEventType() == InputEventType_Pressed)
					{
						Action.CanDispatch = false;
					}
				}
			}
			return false;
		}
	}

}
