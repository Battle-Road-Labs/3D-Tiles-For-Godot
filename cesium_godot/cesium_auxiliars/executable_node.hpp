#ifndef EXECUTABLE_NODE_H
#define EXECUTABLE_NODE_H

// If you are reading this, I want you to know that I do not like inheritance, but we gotta comply with Godot's architecture whilst
// keeping all these impl details away from polluting every single file with a lot of #ifdefs
#if defined (CESIUM_GD_EXT)
class ExecutableNode : public Node {
	GDCLASS(ExecutableNode, Node)
};

	
class ExecutableNode3D : public Node3D {
	GDCLASS(ExecutableNode3D, Node3D)
};
	
class ExecutableMeshInstance3D : public MeshInstance3D {
	GDCLASS(ExecutableMeshInstance3D, MeshInstance3D)
}
#else

#define MAKE_EXE_NODE(name, parentClass) \
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
