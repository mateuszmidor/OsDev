/**
 *   @file: RunThisGuy.h
 *
 *   @date: September 13, 2018
 * @author: Mateusz Midor
 */

#ifndef RUNTHISGUY_H_
#define RUNTHISGUY_H_

#include <functional>

/** RunThisGuy namespace */
namespace rtg {

    /**
    * @brief   Associate handler with an event.
    *          This is to prevent "everyone owns pointer to everyone's else interface" dependency infinite loop ∞
    * @example
    *          MainWindow window;
    *          Application app;
    *
    *          window.onCloseClick = run_this_guy(&Application::doClose, app);
    *
    *          , where window.onCloseClick is defined eg. as:
    *          std::function<void(void)> onCloseClick;
    * @warning This function is a nightmare. Don't look. Believe.
    */
    template <class FuncRet, class... FuncArgs, class Obj>
    std::function<FuncRet(FuncArgs...)> run_this_guy(FuncRet (Obj::*fptr)(FuncArgs...), Obj& obj) {
        return [&obj, fptr](FuncArgs...args) { return (obj.*fptr)(args...); };
    }

}

#endif /* RUNTHISGUY_H_ */


