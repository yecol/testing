/** Copyright 2022 Alibaba Group Holding Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MY_APP_H
#define MY_APP_H

#include "my_app_context.h"

namespace gs {

/**
 * @brief Compute the degree for each vertex.
 *
 * @tparam FRAG_T
 */
template <typename FRAG_T>
class MyApp : public grape::ParallelAppBase<FRAG_T, MyAppContext<FRAG_T>>,
              public grape::ParallelEngine,
              public grape::Communicator {
 public:
  INSTALL_PARALLEL_WORKER(MyApp<FRAG_T>, MyAppContext<FRAG_T>, FRAG_T)
  static constexpr grape::MessageStrategy message_strategy =
      grape::MessageStrategy::kSyncOnOuterVertex;
  static constexpr grape::LoadStrategy load_strategy =
      grape::LoadStrategy::kBothOutIn;
  using vertex_t = typename fragment_t::vertex_t;

  /**
   * @brief Implement your partial evaluation here.
   *
   * @param fragment
   * @param context
   * @param messages
   */
  void PEval(const fragment_t& fragment, context_t& context,
             message_manager_t& messages) {
    messages.InitChannels(thread_num());
    // Implement your partial evaluation here.
    // We put all compute logic in IncEval phase, thus do nothing but force continue.
    messages.ForceContinue();
  }

  void IncEval(const fragment_t& fragment, context_t& context,
               message_manager_t& messages) {
    // superstep
    ++context.step;

    // Process received messages sent by other fragment here.
    {
      messages.ParallelProcess<fragment_t, double>(
          thread_num(), fragment,
          [&context](int tid, vertex_t u, const double& msg) {
            // Implement your logic here.
          });
    }

    // Compute the degree for each vertex, set the result in context
    auto inner_vertices = fragment.InnerVertices();
    ForEach(inner_vertices.begin(), inner_vertices.end(),
            [&context, &fragment](int tid, vertex_t u) {
              context.result[u] =
                  static_cast<uint64_t>(fragment.GetOutgoingAdjList(u).Size() +
                                        fragment.GetIncomingAdjList(u).Size());
            });
  }
};
};  // namespace gs

#endif  // MY_APP_H
