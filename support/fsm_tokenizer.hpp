#pragma once

#include "commondefs.hpp"
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include "any.hpp"
#include "str_util.hpp"

namespace lins {

    template<typename STR_T>
    class fsm_tokenizer {

        enum class parse_states { initial, rule, target };

        using CHAR_T = typename STR_T::value_type;

    public:
        fsm_tokenizer() {
            reset();
        }

        fsm_tokenizer(const STR_T &rules) {
            reset();
            add_rules(rules);
        }

        void reset() {
            parse_state_ = parse_states::initial;
            buffer_.clear();
            state_current_ = STATE_INITIAL;
            state_prev_ = STATE_INITIAL;
            rules_.clear();
            line_ = 0;
            colmn_ = 0;
            current_l_ = 0;
            current_c_ = 0;
            match_cache_.clear();
        }

        INLINE_GETTER_SETTER(CHAR_T, name_for_self_char_prefix, SELF_CHAR_PREFIX)
        INLINE_GETTER_SETTER(CHAR_T, name_for_outer_char_prefix, OUTER_CHAR_PREFIX)
        INLINE_GETTER_SETTER(CHAR_T, name_for_rules_separator, RULES_SEPARATOR)
        INLINE_GETTER_SETTER(CHAR_T, name_for_ruleset_finalizer, RULESET_FINALIZER)
        INLINE_GETTER_SETTER(STR_T, name_for_rule_to_target_transition, RULE_TO_TARGET_TRANSITION)
        INLINE_GETTER_SETTER(STR_T, name_for_state_initial, STATE_INITIAL)
        INLINE_GETTER_SETTER(STR_T, name_for_state_error, STATE_ERROR)
        INLINE_GETTER_SETTER(STR_T, name_for_class_default, CLASS_DEFAULT)
        INLINE_GETTER_SETTER(STR_T, name_for_category_space, CATEGORY_SPACE)
        INLINE_GETTER_SETTER(STR_T, name_for_category_alpha, CATEGORY_ALPHA)
        INLINE_GETTER_SETTER(STR_T, name_for_category_alnum, CATEGORY_ALNUM)
        INLINE_GETTER_SETTER(STR_T, name_for_category_cntrl, CATEGORY_CNTRL)
        INLINE_GETTER_SETTER(STR_T, name_for_category_digit, CATEGORY_DIGIT)
        INLINE_GETTER_SETTER(STR_T, name_for_category_punct, CATEGORY_PUNCT)
        INLINE_GETTER_SETTER(STR_T, name_for_category_hex, CATEGORY_HEX)
        INLINE_GETTER_SETTER(STR_T, name_for_category_oct, CATEGORY_OCT)

        void accept_char(CHAR_T sym) {
            accept(sym);
            current_c_++;
            if(sym == '\n') {
                current_l_++;
                current_c_ = 0;
            }
        }

        void accept_string(const STR_T &in_str) {
            for(size_t i{0}; i < in_str.size(); ++i) {
                accept(in_str[i]);
                current_c_++;
                if(in_str[i] == '\n') {
                    current_l_++;
                    current_c_ = 0;
                }
            }
        }

        void finalize() {
            accept(0);
        }

        void set_listener_delegate(std::function<void(lins::any const &)> lsnr) {
            lsnr_ = lsnr;
        }

        void add_rules(const STR_T &rules) {
            match_cache_.clear();
            STR_T state_name{};
            STR_T symbol_or_class{};
            STR_T next_state{};

            STR_T buf_scope{};

            bool escape{false};

            parse_state_ = parse_states::initial;
            for(auto ch : rules) {
                switch (parse_state_) {
                case parse_states::initial:
                    if(escape) {
                        state_name += ch;
                        escape = false;
                    } else {
                        if(ch == RULES_SEPARATOR) {
                            parse_state_ = parse_states::rule;
                            state_name = lins::str_util::trim(state_name);
                            if(state_name.empty()) {
                                throw std::runtime_error{"error in rules"};
                            }
                        } else if(ch == ESCAPE_CHAR) {
                            escape = true;
                        } else {
                            state_name += ch;
                        }
                    }
                    break;
                case parse_states::rule:
                    if(buf_scope + ch == RULE_TO_TARGET_TRANSITION) {
                        buf_scope.clear();
                        parse_state_ = parse_states::target;
                        if(symbol_or_class.size() > 2) {
                            symbol_or_class = symbol_or_class.substr(0, symbol_or_class.size() - RULE_TO_TARGET_TRANSITION.size() + 1);
                            symbol_or_class = lins::str_util::trim(symbol_or_class);
                            if(symbol_or_class.empty()) {
                                throw std::runtime_error{"error in rules"};
                            }
                        } else {
                            throw std::runtime_error{"error in rules"};
                        }
                    } else {
                        symbol_or_class += ch;
                        buf_scope += ch;
                        buf_scope = buf_scope.size() > RULE_TO_TARGET_TRANSITION.size() - 1 ? buf_scope.substr(buf_scope.size() - (RULE_TO_TARGET_TRANSITION.size() - 1)) : buf_scope;
                    }
                    break;
                case parse_states::target:
                    if(escape) {
                        next_state += ch;
                        escape = false;
                    } else {
                        if(ch == RULES_SEPARATOR) {
                            next_state = lins::str_util::trim(next_state);
                            if(next_state.empty()) {
                                throw std::runtime_error{"error in rules"};
                            }
                            add_rule(lins::str_util::trim(state_name), lins::str_util::trim(symbol_or_class), lins::str_util::trim(next_state));
                            symbol_or_class.clear();
                            next_state.clear();
                            parse_state_ = parse_states::rule;
                        } else if(ch == RULESET_FINALIZER) {
                            parse_state_ = parse_states::initial;
                            next_state = lins::str_util::trim(next_state);
                            if(next_state.empty()) {
                                throw std::runtime_error{"error in rules"};
                            }
                            add_rule(lins::str_util::trim(state_name), lins::str_util::trim(symbol_or_class), lins::str_util::trim(next_state));
                            state_name.clear();
                            symbol_or_class.clear();
                            next_state.clear();
                        } else if(ch == ESCAPE_CHAR) {
                            escape = true;
                        } else {
                            next_state += ch;
                        }
                    }
                    break;
                }
            }
            if(parse_state_ != parse_states::initial || escape) {
                throw std::runtime_error{"error in rules"};
            }
        }

    private:
        std::map<STR_T, std::map<CHAR_T, bool>> match_cache_{};

        bool match(CHAR_T sym, const STR_T &s) {
            auto mc_entry_it{match_cache_.find(s)};
            if(mc_entry_it != match_cache_.end()) {
                auto c_entry_it{mc_entry_it->second.find(sym)};
                if(c_entry_it != mc_entry_it->second.end()) {
                    return c_entry_it->second;
                }
            }

            bool res{false};
            bool not_cond{false};
            if(!s.empty() && s[0] == '\'') {
                STR_T sym_or_class{lins::str_util::unescaper<STR_T>(lins::str_util::trim(lins::str_util::unquote<STR_T>(s, lins::str_util::fltr<STR_T>::cast(std::string{ "'" }), lins::str_util::fltr<STR_T>::cast(std::string{ "'" }))))};
                if(sym_or_class.size() != 1) {
                    throw std::runtime_error{"invalid character for match"};
                }
                return sym == sym_or_class[0];
            } else {
                STR_T sym_or_class{lins::str_util::trim(s)};
                enum class local_states {
                    none,
                    class_body,
                    end
                };
                local_states ls{local_states::none};
                bool spaces{false};
                bool alphas{false};
                bool digits{false};
                bool puncts{false};
                bool cntrl{false};
                bool hex{false};
                bool oct{false};

                bool not_spaces{false};
                bool not_alphas{false};
                bool not_digits{false};
                bool not_puncts{false};
                bool not_cntrl{false};
                bool not_hex{false};
                bool not_oct{false};

                STR_T set{};
                STR_T negative_set{};
                bool local_not_cond{false};
                bool local_esc{false};
                for(std::size_t i{0}; i < sym_or_class.size(); ++i) {
                    if(ls == local_states::end) {
                        break;
                    }
                    CHAR_T cc{sym_or_class[i]};
                    switch (ls) {
                    case local_states::none:
                        if(cc == '!') {
                            not_cond = !not_cond;
                        } else if(cc == '[') {
                            ls = local_states::class_body;
                        }
                        break;
                    case local_states::class_body:
                        if(local_esc) {
                            local_esc = false;
                            enum class escape { no, start, oct, hex, uni4, uni8 };
                            escape esc{escape::start};
                            std::string cur_char_num{};
                            while (esc != escape::no) {
                                switch(esc) {
                                    case escape::start:
                                        if(cc == 'r') { cc = '\r'; esc = escape::no; }
                                        else if(cc == 'n') { cc = '\n'; esc = escape::no; }
                                        else if(cc == 't') { cc = '\t'; esc = escape::no; }
                                        else if(cc == 'b') { cc = '\b'; esc = escape::no; }
                                        else if(cc == 'a') { cc = '\a'; esc = escape::no; }
                                        else if(cc == 'v') { cc = '\v'; esc = escape::no; }
                                        else if(cc == 'f') { cc = '\f'; esc = escape::no; }
                                        else if(cc == 'e') { cc = 0x1b; esc = escape::no; }
                                        else if(cc >= '0' && cc <= '7') {
                                            esc = escape::oct;
                                            cur_char_num += cc;
                                        }
                                        else if(cc == 'x' || cc == 'X') {
                                            esc = escape::hex;
                                        }
                                        else if(cc == 'u') { esc = escape::uni4; }
                                        else if(cc == 'U') { esc = escape::uni8; }
                                        else if(cc == '\\') { esc = escape::no; }
                                        else { esc = escape::no; /*unescape_char(cc);*/ }
                                        break;
                                    case escape::oct:
                                        if(cc != 0 && cc >= '0' && cc <= '7') {
                                            cur_char_num += cc;
                                        } else {
                                            throw std::runtime_error{"octal sequence is not completed"};
                                        }
                                        if(cur_char_num.size() == 3) {
                                            char *end{nullptr};
                                            cc = (CHAR_T)::strtol(cur_char_num.c_str(), &end, 8);
                                        }
                                        break;
                                    case escape::hex:
                                        if(cc != 0 && ((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F'))) {
                                            cur_char_num += cc;
                                        } else {
                                            throw std::runtime_error{"hex sequence is not completed"};
                                        }
                                        if(cur_char_num.size() == 2) {
                                            char *end{nullptr};
                                            cc = (CHAR_T)::strtol(cur_char_num.c_str(), &end, 16);
                                            esc = escape::no;
                                        }
                                        break;
                                    case escape::uni4:
                                        if(cc != 0 && ((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F'))) {
                                            cur_char_num += cc;
                                        } else {
                                            throw std::runtime_error{"unicode sequence is not completed"};
                                        }
                                        if(cur_char_num.size() == 4) {
                                            char* end{nullptr};
                                            cc = (CHAR_T)::strtol(cur_char_num.c_str(), &end, 16);
                                            esc = escape::no;
                                        }
                                        break;
                                    case escape::uni8:
                                        if(cc != 0 && ((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F'))) {
                                            cur_char_num += cc;
                                        } else {
                                            throw std::runtime_error{"unicode sequence is not completed"};
                                        }
                                        if(cur_char_num.size() == 8) {
                                            char* end{nullptr};
                                            cc = (CHAR_T)::strtol(cur_char_num.c_str(), &end, 16);
                                            esc = escape::no;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                                if(esc != escape::no) {
                                    ++i;
                                    if(i < sym_or_class.size()) {
                                        cc = sym_or_class[i];
                                    } else {
                                        return false;
                                    }
                                }
                            }
                        } else if(cc == '\\') {
                            local_esc = true;
                            continue;
                        } else if(cc == '!') {
                            local_not_cond = true;
                            continue;
                        } else if(cc == ']') {
                            ls = local_states::end;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_SPACE)) {
                            if(local_not_cond) {
                                not_spaces = true;
                            } else {
                                spaces = true;
                            }
                            i += CATEGORY_SPACE.size() - 1;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_ALPHA)) {
                            if(local_not_cond) {
                                not_alphas = true;
                            } else {
                                alphas = true;
                            }
                            i += CATEGORY_ALPHA.size() - 1;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_ALNUM)) {
                            if(local_not_cond) not_digits = true; else digits = true;
                            if(local_not_cond) not_alphas = true; else alphas = true;
                            i += CATEGORY_ALNUM.size() - 1;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_CNTRL)) {
                            if(local_not_cond) not_cntrl = true; else cntrl = true;
                            i += CATEGORY_CNTRL.size() - 1;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_DIGIT)) {
                            if(local_not_cond) not_digits = true; else digits = true;
                            i += CATEGORY_DIGIT.size() - 1;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_PUNCT)) {
                            if(local_not_cond) not_puncts = true; else puncts = true;
                            i += CATEGORY_PUNCT.size() - 1;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_HEX)) {
                            if(local_not_cond) not_hex = true; else hex = true;
                            i += CATEGORY_HEX.size() - 1;
                            continue;
                        } else if(lins::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_OCT)) {
                            if(local_not_cond) not_oct = true; else oct = true;
                            i += CATEGORY_OCT.size() - 1;
                            continue;
                        }
                        if(local_not_cond) {
                            negative_set += cc;
                        } else {
                            set += cc;
                        }
                        local_not_cond = false;
                        break;

                    default:
                        break;
                    }
                }

                bool neg_found = false;
                if(not_spaces) { if(lins::str_util::fltr<STR_T>::isspace(sym)) { neg_found = true; } }
                if(not_alphas) { if(lins::str_util::fltr<STR_T>::isalpha(sym)) { neg_found = true; } }
                if(not_digits) { if(lins::str_util::fltr<STR_T>::isdigit(sym)) { neg_found = true; } }
                if(not_puncts) { if(lins::str_util::fltr<STR_T>::ispunct(sym)) { neg_found = true; } }
                if(not_cntrl) { if(lins::str_util::fltr<STR_T>::iscntrl(sym)) { neg_found = true; } }
                if(not_hex) { if(lins::str_util::fltr<STR_T>::isdigit(sym) || (sym >= 'a' && sym <= 'f') || (sym >= 'A' && sym <= 'F')) { neg_found = true; } }
                if(not_oct) { if(sym >= '0' && sym <= '7') { neg_found = true; } }
                for(auto c : negative_set) {
                    if(sym == c) {
                        neg_found = true;
                        break;
                    }
                }

                bool pos_given{false};
                bool pos_found{false};
                if(spaces) { pos_given = true; if(lins::str_util::fltr<STR_T>::isspace(sym)) { pos_found = true; } }
                if(alphas) { pos_given = true; if(lins::str_util::fltr<STR_T>::isalpha(sym)) { pos_found = true; } }
                if(digits) { pos_given = true; if(lins::str_util::fltr<STR_T>::isdigit(sym)) { pos_found = true; } }
                if(puncts) { pos_given = true; if(lins::str_util::fltr<STR_T>::ispunct(sym)) { pos_found = true; } }
                if(cntrl) { pos_given = true; if(lins::str_util::fltr<STR_T>::iscntrl(sym)) { pos_found = true; } }
                if(hex) { pos_given = true; if(lins::str_util::fltr<STR_T>::isdigit(sym) || (sym >= 'a' && sym <= 'f') || (sym >= 'A' && sym <= 'F')) { pos_found = true; } }
                if(oct) { pos_given = true; if(sym >= '0' && sym <= '7') { pos_found = true; } }
                for(auto const &c : set) {
                    pos_given = true;
                    if(sym == c) {
                        pos_found = true;
                        break;
                    }
                }

                res = !neg_found && (pos_found || !pos_given);
            }
            bool fin_res{not_cond ? !res : res};

            match_cache_[s][sym] = fin_res;

            return fin_res;
        }

        inline void add_rule(STR_T const &state_name, STR_T const &symbol_or_class, STR_T const &next_state) {
            rules_[state_name].emplace_back(symbol_or_class, next_state);
            match_cache_.clear();
        }

        static inline bool state_valid(STR_T const &s) {
            return !s.empty();
        }

        inline bool state_final(STR_T const &s) const {
            return !s.empty() && (s[0] == SELF_CHAR_PREFIX || s[0] == OUTER_CHAR_PREFIX);
        }

        inline bool state_initial(STR_T const &s) const {
            return s == STATE_INITIAL;
        }

        void set_state(STR_T const &s) {
            if(state_initial(s)) {
                line_ = current_l_;
                colmn_ = current_c_;
            }
            state_prev_ = state_current_;
            state_current_ = s;

        }

        inline lins::any create_token(STR_T const &token, STR_T const &type) {
            std::map<STR_T, lins::any> o{};
            o[lins::str_util::fltr<STR_T>::cast("token")] = token;
            o[lins::str_util::fltr<STR_T>::cast("type")] = type;
            o[lins::str_util::fltr<STR_T>::cast("line")] = line_;
            o[lins::str_util::fltr<STR_T>::cast("column")] = colmn_;
            return o;
        }

        void accept(CHAR_T sym) {
            auto state_map_it{rules_.find(state_current_)};
            if(state_map_it != rules_.end()) {
                auto const &m{state_map_it->second};
                STR_T default_state{};
                STR_T match_state{};
                std::vector<STR_T> final_states_list{};
                for(auto const &p : m) {
                    auto const &sym_or_class{p.first};
                    auto const &next_state{p.second};
                    if(sym == 0 && state_final(next_state)) {
                        final_states_list.push_back(next_state);
                    }
                    if(sym_or_class == CLASS_DEFAULT) {
                        default_state = next_state;
                    } else if(match_state.empty() && match(sym, sym_or_class)) {
                        match_state = next_state;
                    }
                }

                if(sym == 0) {
                    if(state_current_ != STATE_INITIAL && !buffer_.empty()) {
                        if(!final_states_list.empty()) {
                            if(lsnr_) {
                                lsnr_(create_token(buffer_, final_states_list[0]));
                            }
                        } else {
                            throw std::runtime_error{"unexpected end of input stream"};
                        }
                    }
                    reset();
                    return;
                }

                bool default_present{!default_state.empty()};
                bool match_found{!match_state.empty()};
                bool match_is_final{state_final(match_state)};
                bool default_is_final{state_final(default_state)};

                if(match_state == STATE_ERROR) {
                    std::stringstream ss{};
                    ss << "invalid character at line " << current_l_ + 1 << ", col " << current_c_ + 1;
                    throw std::runtime_error{ss.str()};
                }

                if(match_found) {
                    if(match_is_final) {
                        if(state_initial(state_current_)) {
                            buffer_.clear();
                            buffer_ += sym;
                            if(lsnr_) { lsnr_(create_token(buffer_, match_state)); }
                            buffer_.clear();
                        } else {
                            bool sym_belongs_current{match_state[0] == SELF_CHAR_PREFIX};
                            if(sym_belongs_current) {
                                buffer_ += sym;
                                if(lsnr_) {
                                    lsnr_(create_token(buffer_, match_state));
                                }
                                buffer_.clear();
                                set_state(STATE_INITIAL);
                            } else {
                                if(lsnr_) {
                                    lsnr_(create_token(buffer_, match_state));
                                }
                                buffer_.clear();
                                set_state(STATE_INITIAL);
                                accept(sym);
                            }
                        }
                    } else {
                        buffer_ += sym;
                        set_state(match_state);
                    }
                } else {
                    if(default_present) {
                        if(default_is_final) {
                            bool sym_belongs_current{default_state[0] == SELF_CHAR_PREFIX};
                            if(sym_belongs_current) {
                                buffer_ += sym;
                            }
                            if(lsnr_) {
                                lsnr_(create_token(buffer_, default_state));
                            }
                            buffer_.clear();
                            set_state(STATE_INITIAL);
                            if(!sym_belongs_current) {
                                accept(sym);
                            }
                        } else {
                            set_state(default_state);
                            buffer_ += sym;
                        }
                    } else {
                        std::stringstream ss{};
                        ss << "invalid character at line " << current_l_ + 1 << ", col " << current_c_ + 1;
                        throw std::runtime_error{ss.str()};
                    }
                }
            }
        }

        STR_T RULE_TO_TARGET_TRANSITION{lins::str_util::fltr<STR_T>::cast(">>")};
        STR_T STATE_INITIAL{lins::str_util::fltr<STR_T>::cast("#rd")};
        STR_T STATE_ERROR{lins::str_util::fltr<STR_T>::cast("#er")};
        STR_T CLASS_DEFAULT{lins::str_util::fltr<STR_T>::cast("$df")};
        STR_T CATEGORY_SPACE{lins::str_util::fltr<STR_T>::cast(":space:")};
        STR_T CATEGORY_ALPHA{lins::str_util::fltr<STR_T>::cast(":alpha:")};
        STR_T CATEGORY_ALNUM{lins::str_util::fltr<STR_T>::cast(":alnum:")};
        STR_T CATEGORY_CNTRL{lins::str_util::fltr<STR_T>::cast(":cntrl:")};
        STR_T CATEGORY_DIGIT{lins::str_util::fltr<STR_T>::cast(":digit:")};
        STR_T CATEGORY_PUNCT{lins::str_util::fltr<STR_T>::cast(":punct:")};
        STR_T CATEGORY_HEX{lins::str_util::fltr<STR_T>::cast(":hex:")};
        STR_T CATEGORY_OCT{lins::str_util::fltr<STR_T>::cast(":oct:")};
        CHAR_T ESCAPE_CHAR{'\\'};
        CHAR_T SELF_CHAR_PREFIX{'<'};
        CHAR_T OUTER_CHAR_PREFIX{'^'};
        CHAR_T RULES_SEPARATOR{'|'};
        CHAR_T RULESET_FINALIZER{';'};

        parse_states parse_state_{parse_states::initial};
        STR_T buffer_{};
        STR_T state_prev_{};
        STR_T state_current_{};
        std::map<STR_T, std::list<std::pair<STR_T, STR_T>>> rules_{};
        int line_{0};
        int colmn_{0};
        int current_l_{0};
        int current_c_{0};

        std::function<void(lins::any const &)> lsnr_{nullptr};
    };

}

#undef USE_CUSTOM_HASHER
