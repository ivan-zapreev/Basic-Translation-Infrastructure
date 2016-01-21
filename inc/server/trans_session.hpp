/* 
 * File:   trans_session.hpp
 * Author: Dr. Ivan S. Zapreev
 *
 * Visit my Linked-in profile:
 *      <https://nl.linkedin.com/in/zapreevis>
 * Visit my GitHub:
 *      <https://github.com/ivan-zapreev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.#
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created on January 20, 2016, 3:13 PM
 */

#ifndef TRANS_SESSION_HPP
#define TRANS_SESSION_HPP

namespace uva {
    namespace smt {
        namespace decoding {
            namespace server {

                //Make the typedef for the translation session id
                typedef uint64_t session_id_type;

                namespace session {

                    //Stores the undefined session job id value
                    static constexpr session_id_type UNDEFINED_SESSION_ID = 0;

                    //Stores the minimum allowed session job id
                    static constexpr session_id_type MINIMUM_SESSION_ID = 1;
                }
            }
        }
    }
}


#endif /* TRANS_SESSION_HPP */
