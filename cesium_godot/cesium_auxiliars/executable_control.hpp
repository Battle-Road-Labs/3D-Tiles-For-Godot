#ifndef EXECUTABLE_NODE_H
#define EXECUTABLE_NODE_H


#if defined (CESIUM_GD_EXT)
class ExecutableControl : public Control {
	GDCLASS(ExecutableControl, Control)
};
	
class ExecutableBoxContainer : public BoxContainer {
	GDCLASS(ExecutableBoxContainer, BoxContainer)
};
#else

#define PREPROCESSOR_PRIMITIVE(pound) pound
#define PREPROCESSOR_INSTRUCTION(pound, instruction) pound##instruction


#define PREPROCESSOR_IF #if
#define PREPROCESSOR_IFNDEF #ifndef
#define PREPROCESSOR_END_IF #endif
#define PREPROCESSOR_DEF #define


#define MAKE_EXE_CONTROL(name, parentClass) \
class name : public parentClass { \
	GDCLASS(name, parentClass) \
public: \
	virtual void _enter_tree() { \
	 \
	} \
	 \
	virtual void _exit_tree() { \
	 \
	} \
	 \
	virtual void _ready() { \
	 \
	} \
	 \
	virtual void _draw() { \
	 \
	} \
	 \
	virtual void _process(real_t delta) { \
	 \
	} \
	 \
 \
protected: \
	using parentClass::_notification; \
	void _notification(int p_what) { \
		switch(p_what) { \
			case NOTIFICATION_READY: \
				this->_ready(); \
				break; \
			case NOTIFICATION_PROCESS: \
				this->_process(p_what); \
				break; \
			case NOTIFICATION_ENTER_TREE: \
				this->_enter_tree(); \
				break; \
			case NOTIFICATION_EXIT_TREE: \
				this->_exit_tree(); \
				break; \
			case CanvasItem::NOTIFICATION_DRAW: \
				this->_draw(); \
				break; \
			default: \
				break; \
		} \
		 \
	} \
	 \
	static void _bind_methods() { \
		 \
	} \
	 \
}\


#endif
#endif

