// Copyright Insight Interactive. All Rights Reserved.
/*
	File - Application.h
	Source - Application.cpp

	Author - Garrett Courtney

	Purpose:
	Core application the engine uses to execute.

	Description:
	The application holds the layers that make up the application. Layers give functionality to the application
	whether it be game code, editor code etc. These are the core components of any program using the Insight api.

*/

#pragma once

#include <Runtime/Core.h>

#include "Runtime/Core/Window.h"
#include "Runtime/Core/Scene/Scene.h"
#include "Runtime/Systems/FrameTimer.h"
#include "Runtime/Core/Layer/LayerStack.h"

#include "Runtime/Core/Events/ApplicationEvent.h"
#include "Runtime/Input/InputDispatcher.h"

#include "Runtime/Core/Layer/GameLayer.h"
#include "Runtime/Core/Layer/ImGuiOverlay.h"
#include "Runtime/Core/Layer/EditorOverlay.h"
#include "Runtime/Core/Layer/PerfOverlay.h"



namespace Insight 
{
	namespace Graphics
	{
		class IRenderContext;
	}

	class INSIGHT_API Application
	{
	public:
		typedef enum EErrorCode
		{
			EC_Failed	= 0,
			EC_Success	= 1,
		} EErrorCode;
	public:
		Application();
		virtual ~Application();

		Application(Application& App) = delete;
		Application(Application&& App) = delete;

		inline static Application& Get() { return *s_Instance; }

		void DumpApp();

		Graphics::IRenderContext* m_pRenderContext = NULL;

		// Initialize the core components of the application. Should be called once
		// at the beginning of the application, after the window has been initialized.
		virtual void Initialize();
		// Called when the main portion of the applicaiton has been initialized.
		virtual void PostInit();
		// Main loop of the application. This is the main entry point for every frame.
		virtual EErrorCode Run();
		EErrorCode RunSingleThreaded();
		// Shutdown the application and release all resources.
		virtual void Shutdown();

		void OnEvent(Event& e);
		// Add a layer to the application's layer stack.
		void PushLayer(Layer* layer);
		// Push an overlay to the front of the application's layer stack.
		void PushOverlay(Layer* layer);
		// Sets a reference to the main window to render too.
		inline void SetWindow(std::shared_ptr<Window> pWindow) 
		{ 
			m_pWindow = pWindow; 
			m_InputDispatcher.SetWindowRef(pWindow);
		}
		
		// Returns a reference to the currently active scene.
		inline Scene& GetScene() const { return *(m_pGameLayer->GetScene()); }
		// Get the applications core layer stack.
		inline LayerStack& GetLayerStack() { return m_LayerStack; }
		// Get the ImGui UI layer.
		inline ImGuiLayer& GetImGuiLayer() { return *m_pImGuiLayer; }
		// Get the game layer that handles the update logic for the runtime components.
		// Should only ever be used for editor actions.
		inline GameLayer& GetGameLayer() { return *m_pGameLayer; }
		// Get the editor layer for the application.
		IE_STRIP_FOR_GAME_DIST(inline EditorLayer& GetEditorLayer() { return *m_pEditorLayer; })
		// Returns a reference to the main rendering window assocciated with the application. 
		inline Window& GetWindow() { return *m_pWindow; }
		// Returns a reference to the game threads performance timer.
		inline FrameTimer& GetGameThreadPerfTimer() { return m_GameThreadTimer; }
		// Returns a reference to the graphics thread performance timer
		inline FrameTimer& GetGraphicsThreadPerfTimer() { return m_GraphicsThreadTimer; }

		// Returns true if the editor is currently simmulating a game session.
		inline static bool IsPlaySessionUnderWay() { return s_Instance->m_pGameLayer->IsPlaySessionUnderWay(); }
		inline static const bool& IsApplicationRunning() { return s_Instance->m_Running; }

	private:
		void PushCoreLayers();
		void RenderThread();

		virtual bool OnWindowClose(WindowCloseEvent& e);
		virtual bool OnWindowResize(WindowResizeEvent& e);
		virtual bool OnWindowFullScreen(WindowToggleFullScreenEvent& e);
		bool OnAppSuspendingEvent(AppSuspendingEvent& e);
		bool OnAppResumingEvent(AppResumingEvent& e);

		virtual bool SaveScene(SceneSaveEvent& e);
		virtual bool BeginPlay(AppBeginPlayEvent& e);
		virtual bool EndPlay(AppEndPlayEvent& e);
		virtual bool ReloadScripts(AppScriptReloadEvent& e);
		virtual bool ReloadShaders(ShaderReloadEvent& e);

	protected:
		std::shared_ptr<Window>	m_pWindow;

		IE_STRIP_FOR_GAME_DIST(ImGuiLayer* m_pImGuiLayer = nullptr; )
		IE_STRIP_FOR_GAME_DIST(EditorLayer* m_pEditorLayer = nullptr; )
		PerfOverlay* m_pPerfOverlay = nullptr;
		GameLayer* m_pGameLayer = nullptr;

		bool					m_IsSuspended = false;
		bool					m_Running = true;
		bool					m_AppInitialized = false;
		LayerStack				m_LayerStack;
		FrameTimer				m_GameThreadTimer;
		FrameTimer				m_GraphicsThreadTimer;
		FileSystem				m_FileSystem;
		Input::InputDispatcher	m_InputDispatcher;

	private:
		static Application*		s_Instance;
	};

	// To be defined by client.
	std::unique_ptr<Application> CreateApplication();

}

