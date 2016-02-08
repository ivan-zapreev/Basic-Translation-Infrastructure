/* 
 * File:   tm_basic_model.hpp
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
 * Created on February 8, 2016, 10:01 AM
 */

#ifndef TM_BASIC_MODEL_HPP
#define TM_BASIC_MODEL_HPP

namespace uva {
    namespace smt {
        namespace translation {
            namespace server {
                namespace tm {
                    namespace models {

                        /**
                         * This class represents a basic translation model implementation.
                         */
                        class tm_basic_model {
                        public:

                            /**
                             * The basic class constructor
                             */
                            tm_basic_model() {
                                //ToDo: Implement
                            }

                            /**
                             * Allows to log the model type info
                             */
                            void log_model_type_info() {
                                //ToDo: Implement
                            }
                        protected:
                        private:
                        };
                    }
                }
            }
        }
    }
}


#endif /* TM_BASIC_MODEL_HPP */
