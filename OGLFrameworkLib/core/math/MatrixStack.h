/**
 * @file   MatrixStack.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.19
 *
 * @brief  Implementation of a matrix stack.
 */

#ifndef MATRIXSTACK_H
#define MATRIXSTACK_H

#include <stack>
#include <glm/glm.hpp>

namespace std {

    template<> inline void stack<glm::mat4>::push(glm::mat4&& _Val)
    {
        if (empty()) c.push_back(_Val);
        else c.push_back(top() * _Val);
    }

}

#endif // MATRIXSTACK_H
