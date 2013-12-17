#include "cpprx/rx.hpp"
namespace rx=rxcpp;

#include "catch.hpp"

SCENARIO("select_many", "[select_many][map][operators]"){
    GIVEN("two cold observables. one of ints. one of strings."){
        auto scheduler = std::make_shared<rx::TestScheduler>();
        typedef rx::TestScheduler::Messages<int> m;
        typedef rx::TestScheduler::Messages<std::string> ms;

        long invoked = 0;

        auto xs = scheduler->CreateColdObservable(
            []() {
                m::RecordedT messages[] = {
                    m::OnNext(100, 4),
                    m::OnNext(200, 2),
                    m::OnNext(300, 3),
                    m::OnNext(400, 1),
                    m::OnCompleted(500)
                };
                return m::ToVector(messages);
            }()
        );

        auto ys = scheduler->CreateColdObservable(
            []() {
                ms::RecordedT messages[] = {
                    ms::OnNext(50, "foo"),
                    ms::OnNext(100, "bar"),
                    ms::OnNext(150, "baz"),
                    ms::OnNext(200, "qux"),
                    ms::OnCompleted(250)
                };
                return ms::ToVector(messages);
            }()
        );

        WHEN("each int is mapped to the strings"){

            auto res = scheduler->Start<std::string>(
                [&]() {
                    return observable(from(xs)
                        .select_many([&](int){return ys;}));
                }
            );

            THEN("the output contains strings repeated for each int"){
                ms::RecordedT items[] = {
                    ms::OnNext(350, "foo"),
                    ms::OnNext(400, "bar"),
                    ms::OnNext(450, "foo"),
                    ms::OnNext(450, "baz"),
                    ms::OnNext(500, "bar"),
                    ms::OnNext(500, "qux"),
                    ms::OnNext(550, "baz"),
                    ms::OnNext(550, "foo"),
                    ms::OnNext(600, "bar"),
                    ms::OnNext(600, "qux"),
                    ms::OnNext(650, "baz"),
                    ms::OnNext(650, "foo"),
                    ms::OnNext(700, "qux"),
                    ms::OnNext(700, "bar"),
                    ms::OnNext(750, "baz"),
                    ms::OnNext(800, "qux"),
                    ms::OnCompleted(850)
                };
                auto required = ms::ToVector(items);
                auto actual = res->Messages();
                REQUIRE(required == actual);
            }

            THEN("there was one subscription and one unsubscription to the ints"){
                rx::Subscription items[] = {
                    m::Subscribe(200, 700)
                };
                auto required = m::ToVector(items);
                auto actual = xs->Subscriptions();
                REQUIRE(required == actual);
            }

            THEN("there were four subscription and unsubscription to the strings"){
                rx::Subscription items[] = {
                    ms::Subscribe(300, 550),
                    ms::Subscribe(400, 650),
                    ms::Subscribe(500, 750),
                    ms::Subscribe(600, 850)
                };
                auto required = m::ToVector(items);
                auto actual = ys->Subscriptions();
                REQUIRE(required == actual);
            }
        }
    }
}
