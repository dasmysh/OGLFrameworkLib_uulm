/**
 * @file   GuiHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of some GUI dialogs.
 */

#ifndef GUIHELPER_H
#define GUIHELPER_H

#include "main.h"

namespace cgu {

    class GuiHelper final
    {
    public:
        enum class DialogReturn
        {
            NO_RETURN,
            OK,
            CANCEL
        };
        static std::tuple<DialogReturn, std::string> OpenFileDialog(const std::string& name, bool& showFileDialog);
    };
}


#endif // GUIHELPER_H
