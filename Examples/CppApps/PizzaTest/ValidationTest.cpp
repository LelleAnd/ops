#include "ValidationTest.h"

#ifdef __cpp_lib_stacktrace
#include <stacktrace>            // A C++23 feature
#endif

#include "Publisher.h"
#include "Subscriber.h"

void ValidationTest(std::string& line, ops::Participant& part, int& varValidation)
{
    if (line.size() == 0) { return; }

    // line:
    // x    x=? isValid()|i set invalid|v set valid|n none|e exception|c[stp] callback

    switch (line[0]) {
    case '?':
        //   '?' handled in caller
        break;
    case 'i':
    case 'I': {
        varValidation = -99;
        break;
    }
    case 'v':
    case 'V': {
        varValidation = 1;
        break;
    }
    case 'n':
    case 'N': {
        part.definePublisherValidation(ops::ValidationStrategy::None);
        part.defineSubscriberValidation(ops::ValidationStrategy::None);
        std::cout << "Note: Pub/Sub need to be recreated to use new validation strategy" << std::endl;
        break;
    }
    case 'e':
    case 'E': {
        part.definePublisherValidation(ops::ValidationStrategy::Exception);
        part.defineSubscriberValidation(ops::ValidationStrategy::Exception);
        std::cout << "Note: Pub/Sub need to be recreated to use new validation strategy" << std::endl;
        break;
    }
    case 'c':
    case 'C': {
        // enum class ValidationAction { Skip, Throw, Pass };
        ops::ValidationAction act = ops::ValidationAction::Skip;
        if (line.size() > 1) {
            switch (line[1]) {
            case 's':
            case 'S':
                act = ops::ValidationAction::Skip;
                break;
            case 't':
            case 'T':
                act = ops::ValidationAction::Throw;
                break;
            case 'p':
            case 'P':
                act = ops::ValidationAction::Pass;
                break;
            default:
                std::cout << "Unknown callback action '" << line[1] << "'" << std::endl;
            }
        }

        // Callback signature
        // std::function<ValidationAction(const Publisher* const, const OPSObject* const)>;
        auto cbPub = [act](const ops::Publisher* const pub, const ops::OPSObject* const) -> ops::ValidationAction {
            std::cout << "[ValidationCallback, Publisher] A NOT valid object detected for topic: " << 
                pub->getTopic().getName() << std::endl;

#ifdef __cpp_lib_stacktrace
            if (act == ops::ValidationAction::Throw) { std::cout << std::stacktrace::current() << '\n'; }
#endif
            return act;
            };

        part.definePublisherValidation(ops::ValidationStrategy::Callback, cbPub);

        // Callback signature
        // std::function<ValidationAction(const Subscriber* const, const OPSMessage* const, const OPSObject* const)>;
        auto cbSub = [act](const ops::Subscriber* const sub, const ops::OPSMessage* const message,
                           const ops::OPSObject* const) -> ops::ValidationAction {
            ops::Address_T address;
            uint16_t port;
            message->getSource(address, port);

            std::cout << "[ValidationCallback, Subscriber] A NOT valid object detected for topic: "
                      << sub->getTopic().getName()
                      << ", From: " << address << "::" << port << std::endl;

            return act;
        };

        part.defineSubscriberValidation(ops::ValidationStrategy::Callback, cbSub);

        std::cout << "Note: Pub/Sub need to be recreated to use new validation strategy" << std::endl;
        break;
    }
    default:
        std::cout << "Unknown command 'c" << line[0] << "'" << std::endl;
    }
}