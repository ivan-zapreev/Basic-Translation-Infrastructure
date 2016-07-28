/* 
 * File:   processor_parameters.hpp
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
 * Created on July 25, 2016, 11:41 AM
 */

#ifndef PROCESSOR_PARAMETERS_HPP
#define PROCESSOR_PARAMETERS_HPP

#include <map>
#include <string>

#include "common/messaging/language_registry.hpp"
#include "common/utils/exceptions.hpp"
#include "common/utils/logging/logger.hpp"
#include "common/utils/string_utils.hpp"

using namespace std;

using namespace uva::utils::logging;
using namespace uva::utils::exceptions;
using namespace uva::utils::text;

using namespace uva::smt::bpbd::common::messaging;

namespace uva {
    namespace smt {
        namespace bpbd {
            namespace processor {

                //Forward declaration
                struct language_config_struct;

                //Typedef the structure
                typedef language_config_struct language_config;

                /**
                 * The structure for storing the language config parameters
                 */
                struct language_config_struct {
                    //Stores working dir tempalate parameter name
                    static const string WORK_DIR_TEMPL_PARAM_NAME;
                    //Stores processor job unique id template parameter name
                    static const string JOB_UID_TEMPL_PARAM_NAME;
                    //Stores language template parameter name
                    static const string LANGUAGE_TEMPL_PARAM_NAME;

                    /**
                     * The basic constructor for the structure that allows
                     * to set the reference to the work directory
                     * @param work_dir  the reference to the work directory name
                     */
                    language_config_struct(const string & work_dir)
                    : m_work_dir(work_dir), m_lang(""), m_call_templ("") {
                    }

                    /**
                     * Allows to check if the language configuration is set.
                     * I.e. that there is call template set.
                     * @return true if the call template is set, otherwise false.
                     */
                    inline bool is_defined() const {
                        return !m_call_templ.empty();
                    }

                    /**
                     * Allows to get the pre-processor call for the given job id and language.
                     * Note: This is not very optimal to search each time, can be optimized by
                     * not searching for at least one of the template parameters
                     * @param proc_job_id the job id
                     * @param lang the source language
                     * @return a ready to call string
                     */
                    inline string get_call_string(const string file_name, const string lang) const {
                        if (!m_call_templ.empty()) {
                            string result = m_call_templ;
                            replace(result, WORK_DIR_TEMPL_PARAM_NAME, m_work_dir);
                            replace(result, JOB_UID_TEMPL_PARAM_NAME, file_name);
                            replace(result, LANGUAGE_TEMPL_PARAM_NAME, lang);
                            return result;
                        } else {
                            return "";
                        }
                    }

                    /**
                     * Allows to set the call template, throws in case the
                     * template parameters are not found!
                     * @param lang the language
                     * @param call_templ the call template
                     */
                    inline void set_call_template(string & lang, string & call_templ) {
                        //Store the language
                        m_lang = lang;
                        //Store the template
                        m_call_templ = call_templ;
                        
                        //Check the presence of the parameters
                        check_parameter(WORK_DIR_TEMPL_PARAM_NAME);
                        check_parameter(JOB_UID_TEMPL_PARAM_NAME);
                        check_parameter(LANGUAGE_TEMPL_PARAM_NAME);
                    }

                    /**
                     * Allows to get the work directory name
                     * @return the work directory name
                     */
                    inline const string & get_work_dir() const {
                        return m_work_dir;
                    }

                private:
                    //Stores the reference to the work directory name string
                    const string & m_work_dir;
                    //Stores the language 
                    string m_lang;
                    //Stores the script call template
                    string m_call_templ;

                    /**
                     * Allows to check the presence of the template parameter
                     * place holder in the script call string.
                     * @param param parameter name
                     * @throws uva_exception in case the parameter is not found
                     */
                    inline void check_parameter(const string & param) const {
                        size_t pos = m_call_templ.find(param);
                        ASSERT_CONDITION_THROW(pos == std::string::npos,
                                string("The call template: '") + m_call_templ +
                                string("' does not contain template parameter '") +
                                param + string("'"));
                    }

                    //Add the output stream class as a friend
                    friend std::ostream& operator<<(std::ostream&, const language_config &);
                };

                //Typedef the pointer to the language config
                typedef language_config* language_config_ptr;

                //Typedef the language id to configuration map
                typedef map<language_uid, language_config_ptr> lang_to_conf_map;

                /**
                 * This is the storage for processor parameters:
                 * Responsibilities:
                 *      Store the run-time parameters of the processor application
                 */
                struct processor_parameters_struct {
                    //Stores the configuration section name
                    static const string CONFIG_SECTION_NAME;
                    //Stores the server port parameter name
                    static const string SERVER_PORT_PARAM_NAME;

                    //Stores the number of request threads parameter name
                    static const string NUM_THREADS_PARAM_NAME;

                    //Stores the work directory parameter name
                    static const string WORK_DIR_PARAM_NAME;

                    //Stores the language configurations parameter name
                    static const string LANG_CONFIGS_PARAM_NAME;

                    //The delimiter for the language configuration names
                    static const string LANG_CONFIGS_DELIMITER_STR;

                    //Stores the pre-processor script call template parameter name
                    static const string PRE_CALL_TEMPL_PARAM_NAME;
                    //Stores the post-processor script call template parameter name
                    static const string POST_CALL_TEMPL_PARAM_NAME;

                    //The port to listen to
                    uint16_t m_server_port;

                    //The number of the threads handling the requests
                    size_t m_num_threads;

                    //The work directory for storing input and output files
                    string m_work_dir;

                    //The default pre script call template, if
                    //empty then there is no default script given
                    language_config m_def_pre_config;

                    //The default post script call template, if
                    //empty then there is no default script given
                    language_config m_def_post_config;

                    //Stores the mapping from the language id to its call script template
                    lang_to_conf_map m_pre_configs;

                    //Stores the mapping from the language id to its call script template
                    lang_to_conf_map m_post_configs;

                    /**
                     * The basic constructor
                     */
                    processor_parameters_struct()
                    : m_def_pre_config(m_work_dir), m_def_post_config(m_work_dir) {
                    }

                    /**
                     * The basic destructor
                     */
                    virtual ~processor_parameters_struct() {
                        //Iterate through the maps and delete the language configuration objects
                        for (auto iter = m_pre_configs.begin(); iter != m_pre_configs.end(); ++iter) {
                            delete iter->second;
                        }
                        for (auto iter = m_post_configs.begin(); iter != m_post_configs.end(); ++iter) {
                            delete iter->second;
                        }
                    }

                    /**
                     * Allows to add a new translator config.
                     * @param lang the language, if empty then the call templates
                     *        are for the default pre-/post-processors, is trimmed,
                     *        is converted to lower case.
                     * @param pre_call_templ the pre-processor script call template,
                     *                        if empty then ignored, is trimmed.
                     * @param post_call_templ the post-processor script call template,
                     *                        if empty then ignored, is trimmed.
                     */
                    inline void add_language(string lang, string pre_call_templ, string post_call_templ) {
                        //Trim the string
                        trim(lang);
                        trim(pre_call_templ);
                        trim(post_call_templ);

                        LOG_DEBUG << "Registering: " << lang << " pre: " << pre_call_templ
                                << ", post: " << post_call_templ << END_LOG;

                        //Check if the language is given, if not then it is for the default one
                        if (lang.empty()) {
                            //Register the pre script if not empty
                            if (!pre_call_templ.empty()) {
                                m_def_pre_config.set_call_template(lang, pre_call_templ);
                            }
                            //Register the post script if not empty
                            if (!post_call_templ.empty()) {
                                m_def_post_config.set_call_template(lang, post_call_templ);
                            }
                        } else {
                            //Register the language uid
                            language_uid uid = language_registry::register_uid(lang);

                            //Register the pre script if not empty
                            if (!pre_call_templ.empty()) {
                                //Instantiate a new language config
                                language_config_ptr conf = new language_config(m_work_dir);
                                //Set it with the data
                                conf->set_call_template(lang, pre_call_templ);
                                //Store it in the mapping
                                m_pre_configs[uid] = conf;
                            }

                            //Register the post script if not empty
                            if (!post_call_templ.empty()) {
                                //Instantiate a new language config
                                language_config * conf = new language_config(m_work_dir);
                                //Set it with the data
                                conf->set_call_template(lang, post_call_templ);
                                //Store it in the mapping
                                m_post_configs[uid] = conf;
                            }
                        }
                    }

                    /**
                     * Allows to finalize the parameters after loading.
                     */
                    inline void finalize() {
                        ASSERT_CONDITION_THROW((m_num_threads == 0),
                                string("The number of request threads: ") +
                                to_string(m_num_threads) +
                                string(" must be larger than zero! "));
                    }

                    /**
                     * Allows to get the pre-processor call for the given job id and language.
                     * @param proc_job_id the job id
                     * @param lang the source language
                     * @return a ready to call string or an empty string if none could be found
                     */
                    inline string get_lang_pre_call(string proc_job_id, string lang) {
                        return get_lang_call(proc_job_id, lang, m_pre_configs, m_def_pre_config);
                    }

                    /**
                     * Allows to get the post-processor call for the given job id and language.
                     * @param proc_job_id the job id
                     * @param lang the source language
                     * @return a ready to call string or an empty string if none could be found
                     */
                    inline string get_lang_post_call(string proc_job_id, string lang) {
                        return get_lang_call(proc_job_id, lang, m_post_configs, m_def_post_config);
                    }

                private:

                    /**
                     * Allows to get the pre-processor call for the given job id and language.
                     * @param proc_job_id the job id
                     * @param lang the source language
                     * @param configs the map of known configs
                     * @param def the default config
                     * @return a ready to call string or an empty string if none could be found
                     */
                    static inline string get_lang_call(string proc_job_id, string lang,
                            const lang_to_conf_map & configs, const language_config & def) {
                        //Register the language uid
                        language_uid uid = language_registry::get_uid(lang);

                        //Search for the uid
                        auto iter = configs.find(uid);

                        //If the language was found then we have
                        //a string otherwise try the default
                        if (iter != configs.end()) {
                            //Get the entry to work with
                            const language_config_ptr entry = iter->second;
                            //Return the result
                            return entry->get_call_string(proc_job_id, lang);
                        } else {
                            //Return the default result
                            return def.get_call_string(proc_job_id, lang);
                        }
                    }
                };

                //Typedef the structure
                typedef processor_parameters_struct processor_parameters;

                /**
                 * Allows to output the config object to the stream
                 * @param stream the stream to output into
                 * @param config the config object
                 * @return the stream that we output into
                 */
                std::ostream& operator<<(std::ostream& stream, const language_config & config);

                /**
                 * Allows to output the parameters object to the stream
                 * @param stream the stream to output into
                 * @param params the parameters object
                 * @return the stream that we output into
                 */
                static inline std::ostream& operator<<(std::ostream& stream, const processor_parameters & params) {
                    //Dump the main server config
                    stream << "Processor parameters: {server_port = " << params.m_server_port
                            << ", num_threads = " << params.m_num_threads
                            << ", work_dir = " << params.m_work_dir
                            << ", def_pre_conf = " << params.m_def_pre_config
                            << ", def_post_conf = " << params.m_def_post_config;

                    //Dump the pre-processor configs
                    stream << ", pre_configs: [";
                    for (auto iter = params.m_pre_configs.begin(); iter != params.m_pre_configs.end(); ++iter) {
                        stream << iter->second << ", ";
                    }

                    //Dump the post-processor configs
                    stream << "], post_configs: [";
                    for (auto iter = params.m_post_configs.begin(); iter != params.m_post_configs.end(); ++iter) {
                        stream << iter->second << ", ";
                    }

                    //Finish the dump
                    return stream << "]}";
                }
            }
        }
    }
}


#endif /* PROCESSOR_PARAMETERS_HPP */

