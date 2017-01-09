#ifndef FSM_H
#define FSM_H

#include <vector>
#include <functional>
#include <memory>
#include <cassert>

/// events

struct Event
{
  virtual ~Event()
  {
  }
};

struct InitEvent : public Event
{
};

template<typename DataType>
class DataEvent: public Event
{
public:
  explicit DataEvent(const DataType& data) :
    m_data(data)
  {
  }

  const DataType data() const
  {
    return m_data;
  }

private:
  const DataType m_data;
};

/// transitions

class State;
typedef std::shared_ptr<State> StatePtr;

class AbstractTransition
{
public:
  virtual bool eventTest(const Event& e) const = 0;
  virtual void onTransition(const Event& e) = 0;

  virtual ~AbstractTransition()
  {
  }

  std::shared_ptr<State> targetState() const
  {
    return m_targetState.lock();
  }

  void setTargetState(const std::shared_ptr<State>& targetState)
  {
    m_targetState = targetState;
  }

private:
  std::weak_ptr<State> m_targetState;
};

typedef std::shared_ptr<AbstractTransition> AbstractTransitionPtr;

template<typename EventType>
class Transition: public AbstractTransition
{
public:
  explicit Transition(const StatePtr& targetState = StatePtr())
  {
    this->setTargetState(targetState);
  }

  virtual bool eventTest(const Event& e) const override
  {
    return (typeid(EventType) == typeid(e));
  }
  virtual void onTransition(const Event& e) override
  {
    onTransition(dynamic_cast<const EventType&>(e));
  }
  virtual void onTransition(const EventType& pEvent)
  {
  }
};

template<typename EventType>
class GuardTransition:
  public Transition<EventType>
{
public:
  typedef std::function<void(const EventType&)> CallbackType;
  typedef std::function<bool(const EventType&)> GuardType;

  explicit GuardTransition(const StatePtr& targetState = StatePtr()) : Transition<EventType>(targetState)
  {
  }

  virtual bool eventTest(const Event& e) const override
  {
    if(Transition<EventType>::eventTest(e))
    {
      const EventType& event = dynamic_cast<const EventType&>(e);

      for(const GuardType& when : m_guards)
      {
        if(!when(event))
          return false;
      }
      return true;
    }
    return false;
  }

  virtual void onTransition(const EventType& e) override
  {
    if(m_callback)
      m_callback(e);
  }

  void setCallback(const CallbackType& callback)
  {
    m_callback = callback;
  }

  void addGuard(const GuardType& guard)
  {
    m_guards.push_back(guard);
  }

private:
  CallbackType m_callback;
  std::vector<GuardType> m_guards;
};

/// state

class State
{
public:
  explicit State(bool isInitial = false)
    : m_isInitial(isInitial)
  {
  }

  virtual ~State()
  {
  }

  virtual void onEntry(const Event& e)
  {
  }

  virtual void onExit(const Event& e)
  {
  }

  bool isInitial() const
  {
    return m_isInitial;
  }

  void addTransition(const AbstractTransitionPtr& transition)
  {
    m_outgoingTransitions.push_back(transition);
  }

  AbstractTransitionPtr getActiveTransition(const Event& e)
  {
    for(const AbstractTransitionPtr& transition : m_outgoingTransitions)
    {
      if(transition->eventTest(e))
        return transition;
    }

    return AbstractTransitionPtr();
  }
private:
  const bool m_isInitial;
  std::vector<AbstractTransitionPtr> m_outgoingTransitions;
};

class StateMachine : public State
{
public:
  explicit StateMachine() {}

  void add(const StatePtr& state)
  {
    m_states.push_back(state);

    if(state->isInitial())
      m_activeState = state;
  }

  void start()
  {
    assert(m_activeState);

    if(m_activeState)
      m_activeState->onEntry(InitEvent());
  }

  void processEvent(const Event& e)
  {
    AbstractTransitionPtr transition;

    if(m_activeState)
    {
      AbstractTransitionPtr transition;

      if(transition = m_activeState->getActiveTransition(e))
      {
        doTransition(transition, e);
      }
      else if(transition = State::getActiveTransition(e))
      {
        doTransition(transition, e);
      }
    }
  }

private:
  void doTransition(const AbstractTransitionPtr& transition, const Event& e)
  {
    const StatePtr targetState = transition->targetState();

    if(targetState)
      m_activeState->onExit(e);

    transition->onTransition(e);

    if(targetState)
    {
      m_activeState = targetState;
      m_activeState->onEntry(e);
    }
  }

  std::vector<StatePtr> m_states;
  StatePtr m_activeState;
};

#endif // FSM_H
