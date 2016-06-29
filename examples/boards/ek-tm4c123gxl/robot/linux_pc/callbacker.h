#ifndef CALLBACKER_H
#define CALLBACKER_H

#include <functional>
#include <memory>
#include <map>
#include <list>

class Callbacker
{
	class Callback
	{
	public:
		Callback();
		virtual ~Callback() { }
		virtual void execute() const = 0;
		unsigned getId() { return id; }
	private:
		unsigned id;
		static unsigned nextId;
	};

	template <typename T>
	class CallbackImpl : public Callback
	{
	public:
		CallbackImpl(const T& func) : _func(func){ }

		~CallbackImpl() { }
		void execute() const
		{
			_func();
		}
	private:
		T _func;
	};

public:

    class EventRegisterer
    {
    public:
        template <typename FuncType, typename ObjType>
        EventRegisterer(int event, FuncType func, ObjType& obj)
                        : helper(new ConcreteHelper<FuncType, ObjType>(event, func, obj))
		{

		}

        unsigned registerToCallbacker(Callbacker& callbacker) const;
    private:
        class Helper
        {
        public:
        	Helper() { }
            virtual ~Helper() { }

            virtual unsigned registerWithParams(Callbacker& callbacker) = 0;
        };

        template <typename FuncType, typename ObjType>
        class ConcreteHelper : public Helper
        {
        public:
        	ConcreteHelper(int _event, FuncType _func, ObjType& _obj) :
        		event(_event),
				func(_func),
				obj(_obj)
        	{

        	}

        	unsigned registerWithParams(Callbacker& callbacker)
        	{
                        return callbacker.registerCallback(event, func, obj);
        	}

        private:

        	int event;
        	FuncType func;
        	ObjType& obj;
        };

        std::unique_ptr<Helper> helper;
    };

	Callbacker() { }
	~Callbacker() { }

	template <typename FuncType, typename ObjType>
	unsigned registerCallback(int event, FuncType func, ObjType& obj);

	template <typename FuncType>
	unsigned registerCallback(int event, FuncType func);

	bool deregisterCallback(unsigned callbackId);

protected:

	void execOnEvent(int event);

private:
	std::map<int, std::list<std::shared_ptr<Callback>>> eventCallbackMap;

};

template <typename FuncType, typename ObjType>
unsigned Callbacker::registerCallback(int event, FuncType func, ObjType& obj)
{
	auto it = eventCallbackMap.find(event);

	if(it == eventCallbackMap.end())
		eventCallbackMap[event] = std::list<std::shared_ptr<Callback>>();

	auto funcObj = std::bind(func, std::ref(obj));
	std::shared_ptr<Callback> callbackPtr =
			std::make_shared<CallbackImpl<decltype(funcObj)>>(funcObj);

	eventCallbackMap.at(event).push_back(callbackPtr);

	return callbackPtr->getId();
}

template <typename T>
unsigned Callbacker::registerCallback(int event, T func)
{
	auto it = eventCallbackMap.find(event);

	if(it == eventCallbackMap.end())
		eventCallbackMap[event] = std::list<std::shared_ptr<Callback>>();

	std::function<T> funcObj(func);
	std::shared_ptr<Callback> callbackPtr =
			std::make_shared<CallbackImpl<decltype(funcObj)>>(funcObj);

	eventCallbackMap.at(event).push_back(callbackPtr);

	return callbackPtr->getId();
}



#endif // CALLBACKER_H
