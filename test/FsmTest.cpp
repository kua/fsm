#include "fsm.h"
#include <chrono>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Exactly;
using ::testing::NiceMock;

struct MockState : State
{
  MockState(bool isInit) : State(isInit) {}

  MOCK_METHOD1(onEntry, void(const Event&));
  MOCK_METHOD1(onExit, void(const Event&));
};

typedef DataEvent<std::string> StringEvent;
typedef DataEvent<std::chrono::system_clock::time_point> TimeEvent;

bool testString(const StringEvent& event)
{
  return !event.data().empty();
}

struct TransitionListener
{
  virtual void onTransition(const StringEvent& e) = 0;
  virtual ~TransitionListener() {}
};

struct MockTransitionListener : TransitionListener
{
  MOCK_METHOD1(onTransition, void(const StringEvent&));
};

TEST(FsmTest, plainFsm)
{
  StateMachine machine;
  auto s1 = std::make_shared<MockState>(true);
  auto s2 = std::make_shared<MockState>(false);
  auto s3 = std::make_shared<MockState>(false);

  s1->addTransition(std::make_shared<Transition<StringEvent>>(s2));
  s2->addTransition(std::make_shared<Transition<TimeEvent>>(s3));

  machine.add(s1);
  machine.add(s2);
  machine.add(s3);

  EXPECT_CALL(*s1, onEntry(_)).Times(Exactly(1));
  machine.start();

  EXPECT_CALL(*s1, onExit(_)).Times(Exactly(1));
  EXPECT_CALL(*s2, onEntry(_)).Times(Exactly(1));
  machine.processEvent(StringEvent("test string"));

  EXPECT_CALL(*s2, onExit(_)).Times(Exactly(1));
  EXPECT_CALL(*s3, onEntry(_)).Times(Exactly(1));
  machine.processEvent(TimeEvent(std::chrono::system_clock::now()));
}

TEST(FsmTest, hierarchicalFsm)
{
  auto s1 = std::make_shared<MockState>(true);
  auto s2 = std::make_shared<MockState>(false);

  auto t1 = std::make_shared<GuardTransition<StringEvent>>(s2);
  t1->addGuard(std::bind(&testString, std::placeholders:: _1));

  s1->addTransition(t1);
  s2->addTransition(std::make_shared<GuardTransition<TimeEvent>>(s1));

  StateMachine machine;
  machine.add(s1);
  machine.add(s2);

  MockTransitionListener mockListener;
  auto t2 = std::make_shared<GuardTransition<StringEvent>>();
  t2->addGuard(std::bind(std::logical_not<bool>(), std::bind(&testString, std::placeholders::_1)));

  t2->setCallback(std::bind(&MockTransitionListener::onTransition, &mockListener, std::placeholders::_1));
  machine.addTransition(t2);

  EXPECT_CALL(*s1, onEntry(_)).Times(Exactly(2));
  EXPECT_CALL(*s1, onExit(_)).Times(Exactly(1));
  EXPECT_CALL(*s2, onEntry(_)).Times(Exactly(1));
  EXPECT_CALL(*s2, onExit(_)).Times(Exactly(1));
  EXPECT_CALL(mockListener, onTransition(_)).Times(Exactly(1));

  machine.start();

  machine.processEvent(StringEvent("test string"));
  machine.processEvent(TimeEvent(std::chrono::system_clock::now()));
  machine.processEvent(StringEvent(""));
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}


