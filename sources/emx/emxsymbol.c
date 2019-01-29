#define EMXSYMBOL_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emxsymbol.c,v 30.9 1994/09/14 00:31:58 marcs Exp $
 *
 *========================================================================*/
/*
    EMX - Keystrokes, functions, and macros

    Symbol tables, and keymap setup.
*/

#include "emx.h"

#if defined(hpux)
#include <X11/HPkeysym.h>
#endif


static int  modifiers[] = {
	0, 
	ShiftMask, 
	ControlMask, 
	KEscapeMask, 
	KCtlxMask,
	KCtlxMask|ControlMask, 
	ControlMask|ShiftMask,
	KEscapeMask|ControlMask, 
	Mod1Mask};

#define MODIFIERS 9



/* Strings for use in the tables */

static char F_abort[] =			"abort";
static char F_all_win_down[] =		"all-win-down";
static char F_all_win_up[] =		"all-win-up";
static char F_auto_back[] =		"auto-back";
static char F_auto_match[] =		"auto-match";
static char F_auto_save[] =		"auto-save";
static char F_back_buffer[] =		"back-buffer";
static char F_back_char[] =		"back-char";
static char F_back_del_char[] =		"back-del-char";
static char F_back_del_word[] =		"back-del-word";
static char F_back_i_search[] =		"back-i-search";
static char F_back_line[] =		"back-line";
static char F_back_line6[] =		"back-line6";
static char F_back_match[] =		"back-match";
static char F_back_page[] =		"back-page";
static char F_back_para[] =		"back-para";
static char F_back_search[] =		"back-search";
static char F_back_tab[] =		"back-tab";
static char F_back_window[] =		"back-window";
static char F_back_word[] =		"back-word";
static char F_bind_macro[] =		"bind-macro";
static char F_bind_to_key[] =		"bind-to-key";
static char F_buffer_name[] =		"buffer-name";
static char F_c_mode[] =		"c-mode";
static char F_cap_word[] =		"cap-word";
static char F_case_sense[] =		"case-sense";
static char F_cd[] =			"cd";
static char F_copy_line[] =		"copy-line";
static char F_copy_region[] =		"copy-region";
static char F_copy_region_to_macro[] =	"copy-region-to-macro";
static char F_copy_word[] =		"copy-word";
static char F_cursor_type[] =		"cursor-type";
static char F_cut_selected[] =		"cut-selected";
static char F_dedent[] =		"dedent";
static char F_default_back[] =		"default-back";
static char F_del_blank_lines[] =	"del-blank-lines";
static char F_del_blanks[] =		"del-blanks";
static char F_delete_curbuf[] =		"delete-curbuf";
static char F_delete_line[] =		"delete-line";
static char F_describe_key[] =		"describe-key";
static char F_display_bindings[] =	"display-bindings";
static char F_display_buffers[] =	"display-buffers";
static char F_display_commands[] =	"display-commands";
static char F_display_position[] =	"display-position";
static char F_display_version[] =	"display-version";
static char F_down_window[] =		"down-window";
static char F_drag_button[] =		"drag-button";
static char F_end_macro[] =		"end-macro";
static char F_enlarge_window[] =	"enlarge-window";
static char F_execute_buffer[] =	"execute-buffer";
static char F_execute_file[] =		"execute-file";
static char F_execute_macro[] =		"execute-macro";
static char F_exit_flush_all[] =	"exit-flush-all";
static char F_extended_command[] =	"extended-command";
static char F_file_append[] =		"file-append";
static char F_file_delete[] =		"file-delete";
static char F_file_read[] =		"file-read";
static char F_file_reload[] =		"file-reload";
static char F_file_save[] =		"file-save";
static char F_file_visit[] =		"file-visit";
static char F_file_write[] =		"file-write";
static char F_fill_paragraph[] =	"fill-paragraph";
static char F_flush_buffers[] =		"flush-buffers";
static char F_forw_buffer[] =		"forw-buffer";
static char F_forw_char[] =		"forw-char";
static char F_forw_del_char[] =		"forw-del-char";
static char F_forw_del_word[] =		"forw-del-word";
static char F_forw_i_search[] =		"forw-i-search";
static char F_forw_line[] =		"forw-line";
static char F_forw_line6[] =		"forw-line6";
static char F_forw_match[] =		"forw-match";
static char F_forw_page[] =		"forw-page";
static char F_forw_para[] =		"forw-para";
static char F_forw_search[] =		"forw-search";
static char F_forw_search_all[] =	"forw-search-all";
static char F_forw_window[] =		"forw-window";
static char F_forw_word[] =		"forw-word";
static char F_goto_bob[] =		"goto-bob";
static char F_goto_bol[] =		"goto-bol";
static char F_goto_eob[] =		"goto-eob";
static char F_goto_eol[] =		"goto-eol";
static char F_goto_line[] =		"goto-line";
static char F_help[] =			"help";
static char F_indent[] =		"indent";
static char F_ins_nl[] =		"ins-nl";
static char F_ins_nl_and_backup[] =	"ins-nl-and-backup";
static char F_ins_nl_and_indent[] =	"ins-nl-and-indent";
static char F_ins_toggle[] =		"ins-toggle";
static char F_insert_file[] =		"insert-file";
static char F_insert_tab[] =		"insert-tab";
static char F_insert_text[] =		"insert-text";
static char F_invalid_key[] =		"invalid-key";
static char F_kill_buffer[] =		"kill-buffer";
static char F_kill_line[] =		"kill-line";
static char F_kill_paragraph[] =	"kill-paragraph";
static char F_kill_region[] =		"kill-region";
static char F_line_shell_cmd[] =	"line-shell-cmd";
static char F_load_bindings[] =		"load-bindings";
static char F_lower_region[] =		"lower-region";
static char F_lower_word[] =		"lower-word";
static char F_make_bound_macro[] =	"make-bound-macro";
static char F_make_macro[] =		"make-macro";
static char F_move_to_last_match[] =	"move-to-last-match";
static char F_numeric_arg[] =		"numeric-arg";
static char F_only_window[] =		"only-window";
static char F_original_bindings[] =	"original-bindings";
static char F_paste_primary[] =		"paste-primary";
static char F_press_paste[] =		"press-paste";
static char F_press_select[] =		"press-select";
static char F_push_newlines[] =		"push-newlines";
static char F_put_date[] =		"put-date";
static char F_put_line_number[] =	"put-line-number";
static char F_put_time[] =		"put-time";
static char F_put_userid[] =		"put-userid";
static char F_query_replace[] =		"query-replace";
static char F_query_replace_all[] =	"query-replace-all";
static char F_quit[] =			"quit";
static char F_quote[] =			"quote";
static char F_realtab[] =		"realtab";
static char F_refresh[] =		"refresh";
static char F_regdedent[] =		"regdedent";
static char F_regindent[] =		"regindent";
static char F_release_select[] =	"release-select";
static char F_reposition_window[] =	"reposition-window";
static char F_reset_bindings[] =	"reset-bindings";
static char F_save_macro[] =		"save-macro";
static char F_save_to_buffer[] =	"save-to-buffer";
static char F_scroll_left[] =		"scroll-left";
static char F_scroll_right[] =		"scroll-right";
static char F_scroll_toggle[] =		"scroll-toggle";
static char F_search_again[] =		"search-again";
static char F_search_for_match[] =	"search-for-match";
static char F_select_back_char[] =	"select-back-char";
static char F_select_back_line[] =	"select-back-line";
static char F_select_back_word[] =	"select-back-word";
static char F_select_bob[] =		"select-bob";
static char F_select_bol[] =		"select-bol";
static char F_select_eob[] =		"select-eob";
static char F_select_eol[] =		"select-eol";
static char F_select_forw_char[] =	"select-forw-char";
static char F_select_forw_line[] =	"select-forw-line";
static char F_select_forw_word[] =	"select-forw-word";
static char F_select_line[] =		"select-line";
static char F_select_region[] =		"select-region";
static char F_self_insert[] =		"self-insert";
static char F_set_file_name[] =		"set-file-name";
static char F_set_fill_column[] =	"set-fill-column";
static char F_set_fill_double[] =	"set-fill-double";
static char F_set_mark[] =		"set-mark";
static char F_set_size[] =		"set-size";
static char F_set_tab_width[] =		"set-tab-width";
static char F_shrink_window[] =		"shrink-window";
static char F_split_visit[] =		"split-visit";
static char F_split_window[] =		"split-window";
static char F_st_mode[] =		"st-mode";
static char F_start_macro[] =		"start-macro";
static char F_swap_dot_and_mark[] =	"swap-dot-and-mark";
static char F_toggle_pc_file[] =	"toggle-pc-file";
static char F_toggle_tabs_on_indent[] = "toggle-tabs-on-indent";
static char F_trim_buffer[] =		"trim-buffer";
static char F_trim_line[] =		"trim-line";
static char F_trim_output_toggle[] =    "trim-output-toggle";
static char F_twiddle[] =		"twiddle";
static char F_type_macro[] =		"type-macro";
static char F_unbind_key[] =		"unbind-key";
static char F_unmodify[] =		"unmodify";
static char F_unselect_all[] =		"unselect-all";
static char F_up_window[] =		"up-window";
static char F_upper_region[] =		"upper-region";
static char F_upper_word[] =		"upper-word";
static char F_use_buffer[] =		"use-buffer";
static char F_view_file[] =		"view-file";
static char F_word_wrap[] =		"word-wrap";
static char F_yank[] =			"yank";
static char F_yank_from_buffer[] =	"yank-from-buffer";


/* Short forms for standard literals */
#define MOD SYMFUNC|SYMMODIFY,	NULL, NULL, NULL
#define reg SYMFUNC,		NULL, NULL, NULL


/* Function table - all pre-defined functions in alphabetical order */
static FUNCTION functions[] =
{
{F_abort,		emxabort,		reg },
{F_all_win_down,	emxallwindown,		reg },
{F_all_win_up,		emxallwinup,		reg },
{F_auto_back,		emxautoback,		reg },
{F_auto_match,		emxautomatch,		reg },
{F_auto_save,		emxautosave,		reg },
{F_back_buffer,		emxbackbuffer,		reg },
{F_back_char,		emxbackchar,		reg },
{F_back_del_char,	emxbackdel,		MOD },
{F_back_del_word,	emxbackdelword,		MOD },
{F_back_i_search,	emxbackisearch,		reg },
{F_back_line,		emxbackline,		reg },
{F_back_line6,		emxbackline6,		reg },
{F_back_match,		emxbackmatch,		reg },
{F_back_page,		emxbackpage,		reg },
{F_back_para,		emxbackpara,		reg },
{F_back_search,		emxbacksearch,		reg },
{F_back_tab,		emxbacktab,		reg },
{F_back_window,		emxbackwindow,		reg },
{F_back_word,		emxbackword,		reg },
{F_bind_macro,		emxbindmacro,		reg },
{F_bind_to_key,		emxbindtokey,		reg },
{F_buffer_name,		emxbuffername,		reg },
{F_c_mode,		emxcmode,		reg },
{F_cap_word,		emxcapword,		MOD },
{F_case_sense,		emxcasesense,		reg },
{F_cd,			emxcd,			reg },
{F_copy_line,		emxcopyline,		reg },
{F_copy_region,		emxcopyregion,		reg },
{F_copy_region_to_macro,emxcopyregiontomacro,	reg },
{F_copy_word,		emxcopyword,		reg },
{F_cursor_type,		emxcursortype,		reg },
{F_cut_selected,	emxcutselected,		MOD },
{F_dedent,		emxdedent,		MOD },
{F_default_back,	emxdefaultback,		reg },
{F_del_blank_lines,	emxdelblanklines,	MOD },
{F_del_blanks,		emxdelblanks,		MOD },
{F_delete_curbuf,	emxdeletecurbuf,	reg },
{F_delete_line,		emxdeleteline,		MOD },
{F_describe_key,	emxdescribekey,		reg },
{F_display_bindings,	emxdisplaybindings,	reg },
{F_display_buffers,	emxdisplaybuffers,	reg },
{F_display_commands,	emxdisplaycommands,	reg },
{F_display_position,	emxdisplayposition,	reg },
{F_display_version,	emxdisplayversion,	reg },
{F_down_window,		emxdownwindow,		reg },
{F_drag_button,		emxdragbutton,		reg },
{F_end_macro,		emxendmacro,		reg },
{F_enlarge_window,	emxenlargewindow,	reg },
{F_execute_buffer,	emxexecutebuffer,	reg },
{F_execute_file,	emxexecutefile,		reg },
{F_execute_macro,	emxexecutemacro,	reg },
{F_exit_flush_all,	emxexitflushall,	reg },
{F_extended_command,	emxextendedcommand,	reg },
{F_file_append,		emxfileappend,		reg },
{F_file_delete,		emxfiledelete,		reg },
{F_file_read,		emxfileread,		MOD },
{F_file_reload,		emxfilereload,		reg },
{F_file_save,		emxfilesave,		reg },
{F_file_visit,		emxfilevisit,		reg },
{F_file_write,		emxfilewrite,		reg },
{F_fill_paragraph,	emxfillparagraph,	MOD },
{F_flush_buffers,	emxflushbuffers,	reg },
{F_forw_buffer,		emxforwbuffer,		reg },
{F_forw_char,		emxforwchar,		reg },
{F_forw_del_char,	emxforwdel,		MOD },
{F_forw_del_word,	emxforwdelword,		MOD },
{F_forw_i_search,	emxforwisearch,		reg },
{F_forw_line,		emxforwline,		reg },
{F_forw_line6,		emxforwline6,		reg },
{F_forw_match,		emxforwmatch,		reg },
{F_forw_page,		emxforwpage,		reg },
{F_forw_para,		emxforwpara,		reg },
{F_forw_search,		emxforwsearch,		reg },
{F_forw_search_all,	emxforwsearchall,	reg },
{F_forw_window,		emxforwwindow,		reg },
{F_forw_word,		emxforwword,		reg },
{F_goto_bob,		emxgotobob,		reg },
{F_goto_bol,		emxgotobol,		reg },
{F_goto_eob,		emxgotoeob,		reg },
{F_goto_eol,		emxgotoeol,		reg },
{F_goto_line,		emxgotoline,		reg },
{F_help,		emxhelp,		reg },
{F_indent,		emxindent,		MOD },
{F_ins_nl,		emxnewline,		MOD },
{F_ins_nl_and_backup,	emxinsnlandbackup,	MOD },
{F_ins_nl_and_indent,	emxinsnlandindent,	MOD },
{F_ins_toggle,		emxinstoggle,		reg },
{F_insert_file,		emxinsertfile,		MOD },
{F_insert_tab,		emxinserttab,		MOD },
{F_insert_text,		emxinserttext,		MOD },
{F_invalid_key,		(FUNC) emxhoot,		reg },
{F_kill_buffer,		emxkillbuffer,		reg },
{F_kill_line,		emxkillline,		MOD },
{F_kill_paragraph,	emxkillparagraph,	MOD },
{F_kill_region,		emxkillregion,		MOD },
{F_line_shell_cmd,	emxlineshellcmd,	MOD },
{F_load_bindings,	emxloadbindings,	reg },
{F_lower_region,	emxlowerregion,		MOD },
{F_lower_word,		emxlowerword,		MOD },
{F_make_bound_macro,	emxmakeboundmacro,	reg },
{F_make_macro,		emxmakemacro,		reg },
{F_move_to_last_match,	emxmovetolastmatch,	reg },
{F_numeric_arg,		emxnumericarg,		reg },
{F_only_window,		emxonlywindow,		reg },
{F_original_bindings,	emxoriginalbindings,	reg },
{F_paste_primary,	emxpasteprimary,	MOD },
{F_press_paste,		emxpresspaste,		MOD },
{F_press_select,	emxpressselect,		reg },
{F_push_newlines,	emxpushnewlines,	reg },
{F_put_date,		emxputdate,		MOD },
{F_put_line_number,	emxputlinenumber,	MOD },
{F_put_time,		emxputtime,		MOD },
{F_put_userid,		emxputuserid,		MOD },
{F_query_replace,	emxqueryreplace,	MOD },
{F_query_replace_all,	emxqueryreplaceall,	MOD },
{F_quit,		emxquit,		reg },
{F_quote,		emxquote,		MOD },
{F_realtab,		emxrealtab,		MOD },
{F_refresh,		emxrefresh,		reg },
{F_regdedent,		emxregdedent,		MOD },
{F_regindent,		emxregindent,		MOD },
{F_release_select,	emxreleaseselect,	reg },
{F_reposition_window,	emxrepositionwindow,	reg },
{F_reset_bindings,	emxresetbindings,	reg },
{F_save_macro,		emxsavemacro,		reg },
{F_save_to_buffer,	emxsavetobuffer,	reg },
{F_scroll_left,		emxscrollleft,		reg },
{F_scroll_right,	emxscrollright,		reg },
{F_scroll_toggle,	emxscrolltoggle,	reg },
{F_search_again,	emxsearchagain,		reg },
{F_search_for_match,	emxsearchformatch,	reg },
{F_select_back_char,	emxselectbackchar,	reg },
{F_select_back_line,	emxselectbackline,	reg },
{F_select_back_word,	emxselectbackword,	reg },
{F_select_bob,		emxselectbob,		reg },
{F_select_bol,		emxselectbol,		reg },
{F_select_eob,		emxselecteob,		reg },
{F_select_eol,		emxselecteol,		reg },
{F_select_forw_char,	emxselectforwchar,	reg },
{F_select_forw_line,	emxselectforwline,	reg },
{F_select_forw_word,	emxselectforwword,	reg },
{F_select_line,		emxselectline,		reg },
{F_select_region,	emxselectregion,	reg },
{F_self_insert,		emxselfinsert,		MOD },
{F_set_file_name,	emxfilename,		MOD },
{F_set_fill_column,	emxsetfillcolumn,	reg },
{F_set_fill_double,	emxsetfilldouble,	reg },
{F_set_mark,		emxsetmark,		reg },
{F_set_size,		emxsetsize,		reg },
{F_set_tab_width,	emxsettabwidth,		reg },
{F_shrink_window,	emxshrinkwindow,	reg },
{F_split_visit,		emxsplitvisit,		reg },
{F_split_window,	emxsplitwindow,		reg },
{F_st_mode,		emxstmode,		reg },
{F_start_macro,		emxstartmacro,		reg },
{F_swap_dot_and_mark,	emxswapdotandmark,	reg },
{F_toggle_pc_file,	emxtogglepcfile,	MOD },
{F_toggle_tabs_on_indent,emxtoggletabsonindent, reg },
{F_trim_buffer,		emxtrimbuffer,		MOD },
{F_trim_line,		emxtrimline,		MOD },
{F_trim_output_toggle,  emxtrimoutputtoggle,	reg },
{F_twiddle,		emxtwiddle,		MOD },
{F_type_macro,		emxtypemacro,		MOD },
{F_unbind_key,		emxunbindkey,		reg },
{F_unmodify,		emxunmodify,		reg },
{F_unselect_all,	emxunselectall,		reg },
{F_up_window,		emxupwindow,		reg },
{F_upper_region,	emxupperregion,		MOD },
{F_upper_word,		emxupperword,		MOD },
{F_use_buffer,		emxusebuffer,		reg },
{F_view_file,		emxviewfile,		reg },
{F_word_wrap,		emxwordwrap,		reg },
{F_yank,		emxyank,		MOD },
{F_yank_from_buffer,	emxyankfrombuffer,	MOD },
};

#define NFUNCS (sizeof(functions) / sizeof(FUNCTION))


/* Local defines for table prettiness */
#define C ControlMask
#define S ShiftMask
#define E KEscapeMask
#define X KCtlxMask
#define M Mod1Mask

#define FP   FUNCTION *

/* Initial bound key table */
static KEY keys[] =
{
{' ',		C,	(FP)F_set_mark,		  NULL, NULL},
{' ',		E,	(FP)F_set_mark,		  NULL, NULL},
{'@',		C,	(FP)F_set_mark,		  NULL, NULL},
{'^',		X,	(FP)F_enlarge_window,	  NULL, NULL},
{'!',		E,	(FP)F_reposition_window,  NULL, NULL},
{'%',		E,	(FP)F_query_replace,	  NULL, NULL},
{'(',		X,	(FP)F_start_macro,	  NULL, NULL},
{')',		X,	(FP)F_end_macro,	  NULL, NULL},
{'+',		E,	(FP)F_forw_buffer,	  NULL, NULL},
{',',		E,	(FP)F_back_match,	  NULL, NULL},
{'-',		E,	(FP)F_back_buffer,	  NULL, NULL},
{'.',		E,	(FP)F_forw_match,	  NULL, NULL},
{'1',		X,	(FP)F_only_window,	  NULL, NULL},
{'1',		E,	(FP)F_select_region,	  NULL, NULL},
{'2',		X,	(FP)F_split_window,	  NULL, NULL},
{'2',		E,	(FP)F_unselect_all,	  NULL, NULL},
{'<',		E,	(FP)F_goto_bob,		  NULL, NULL},
{'<',		X,	(FP)F_scroll_left,	  NULL, NULL},
{'=',		X,	(FP)F_display_position,	  NULL, NULL},
{'=',		E,	(FP)F_forw_buffer,	  NULL, NULL},
{'=',		C,	(FP)F_put_line_number,	  NULL, NULL},
{'>',		E,	(FP)F_goto_eob,		  NULL, NULL},
{'>',		X,	(FP)F_scroll_right,	  NULL, NULL},
{'?',		E,	(FP)F_help,		  NULL, NULL},
{'\\',		E|C,	(FP)F_regindent,	  NULL, NULL},
{'[',		E,	(FP)F_back_para,	  NULL, NULL},
{']',		E,	(FP)F_forw_para,	  NULL, NULL},
{'~',		E,	(FP)F_unmodify,		  NULL, NULL},
{'a',		C,	(FP)F_goto_bol,		  NULL, NULL},
{'a',		E|C,	(FP)F_auto_save,	  NULL, NULL},
{'a',		X|C,	(FP)F_file_append,	  NULL, NULL},
{'b',		X,	(FP)F_use_buffer,	  NULL, NULL},
{'b',		X|C,	(FP)F_display_buffers,	  NULL, NULL},
{'b',		C,	(FP)F_back_char,	  NULL, NULL},
{'b',		E,	(FP)F_back_word,	  NULL, NULL},
{'b',		E|C,	(FP)F_buffer_name,	  NULL, NULL},
{'c',		X|C,	(FP)F_quit,		  NULL, NULL},
{'c',		E,	(FP)F_cap_word,		  NULL, NULL},
{'d',		X,	(FP)F_put_date,		  NULL, NULL},
{'d',		C,	(FP)F_forw_del_char,	  NULL, NULL},
{'d',		E,	(FP)F_forw_del_word,	  NULL, NULL},
{'e',		X,	(FP)F_execute_macro,	  NULL, NULL},
{'e',		X|C,	(FP)F_exit_flush_all,	  NULL, NULL},
{'e',		C,	(FP)F_goto_eol,		  NULL, NULL},
{'f',		X,	(FP)F_set_fill_column,	  NULL, NULL},
{'f',		X|C,	(FP)F_file_visit,	  NULL, NULL},
{'f',		C,	(FP)F_forw_char,	  NULL, NULL},
{'f',		E,	(FP)F_forw_word,	  NULL, NULL},
{'g',		X,	(FP)F_goto_line,	  NULL, NULL},
{'g',		C,	(FP)F_abort,		  NULL, NULL},
{'g',		E,	(FP)F_abort,		  NULL, NULL},
{'h',		C,	(FP)F_help,		  NULL, NULL},
{'i',		X,	(FP)F_ins_toggle,	  NULL, NULL},
{'i',		X|C,	(FP)F_insert_file,	  NULL, NULL},
{'i',		E,	(FP)F_insert_tab,	  NULL, NULL},
{'i',		E|C,	(FP)F_set_tab_width,	  NULL, NULL},
{'j',		C,	(FP)F_ins_nl_and_indent,  NULL, NULL},
{'k',		X,	(FP)F_kill_buffer,	  NULL, NULL},
{'k',		X|C,	(FP)F_case_sense,	  NULL, NULL},
{'k',		C,	(FP)F_kill_line,	  NULL, NULL},
{'l',		X,	(FP)F_execute_file,	  NULL, NULL},
{'l',		X|C,	(FP)F_lower_region,	  NULL, NULL},
{'l',		C,	(FP)F_refresh,		  NULL, NULL},
{'l',		E,	(FP)F_lower_word,	  NULL, NULL},
{'l',		M,	(FP)F_load_bindings,	  NULL, NULL},
{'m',		X|C,	(FP)F_flush_buffers,	  NULL, NULL},
{'m',		C,	(FP)F_ins_nl,		  NULL, NULL},
{'m',		E,	(FP)F_auto_match,	  NULL, NULL},
{'n',		C,	(FP)F_forw_line,	  NULL, NULL},
{'o',		X,	(FP)F_forw_window,	  NULL, NULL},
{'o',		X|C,	(FP)F_del_blank_lines,	  NULL, NULL},
{'o',		C,	(FP)F_ins_nl_and_backup,  NULL, NULL},
{'p',		X,	(FP)F_back_window,	  NULL, NULL},
{'p',		C,	(FP)F_back_line,	  NULL, NULL},
{'p',		E,	(FP)F_move_to_last_match, NULL, NULL},
{'p',		M,	(FP)F_toggle_pc_file,	  NULL, NULL},
{'q',		C,	(FP)F_quote,		  NULL, NULL},
{'q',		E,	(FP)F_fill_paragraph,	  NULL, NULL},
{'r',		X|C,	(FP)F_file_read,	  NULL, NULL},
{'r',		C,	(FP)F_back_i_search,	  NULL, NULL},
{'r',		E,	(FP)F_back_search,	  NULL, NULL},
{'s',		X|C,	(FP)F_file_save,	  NULL, NULL},
{'s',		C,	(FP)F_forw_i_search,	  NULL, NULL},
{'s',		E,	(FP)F_forw_search,	  NULL, NULL},
{'t',		X,	(FP)F_put_time,		  NULL, NULL},
{'t',		C,	(FP)F_twiddle,		  NULL, NULL},
{'u',		X|C,	(FP)F_upper_region,	  NULL, NULL},
{'u',		C,	(FP)F_numeric_arg,	  NULL, NULL},
{'u',		E,	(FP)F_upper_word,	  NULL, NULL},
{'v',		X,	(FP)F_view_file,	  NULL, NULL},
{'v',		X|C,	(FP)F_file_visit,	  NULL, NULL},
{'v',		C,	(FP)F_forw_page,	  NULL, NULL},
{'v',		E,	(FP)F_back_page,	  NULL, NULL},
{'v',		E|C,	(FP)F_display_version,	  NULL, NULL},
{'w',		X,	(FP)F_word_wrap,	  NULL, NULL},
{'w',		X|C,	(FP)F_file_write,	  NULL, NULL},
{'w',		C,	(FP)F_kill_region,	  NULL, NULL},
{'w',		E,	(FP)F_copy_region,	  NULL, NULL},
{'w',		E|C,	(FP)F_kill_paragraph,	  NULL, NULL},
{'x',		X|C,	(FP)F_swap_dot_and_mark,  NULL, NULL},
{'x',		E,	(FP)F_extended_command,	  NULL, NULL},
{'y',		C,	(FP)F_yank,		  NULL, NULL},
{'z',		X,	(FP)F_enlarge_window,	  NULL, NULL},
{'z',		X|C,	(FP)F_shrink_window,	  NULL, NULL},
{'z',		C,	(FP)F_down_window,	  NULL, NULL},
{'z',		E,	(FP)F_up_window,	  NULL, NULL},
{XK_BackSpace,	0,	(FP)F_back_del_char,	  NULL, NULL},
{XK_BackSpace,	E,	(FP)F_back_del_word,	  NULL, NULL},
{XK_Cancel,	0,	(FP)F_abort,		  NULL, NULL},
{XK_Delete,	0,	(FP)F_forw_del_char,	  NULL, NULL},
{XK_Delete,	C,	(FP)F_delete_line,	  NULL, NULL},
{XK_Delete,	S,	(FP)F_forw_del_word,	  NULL, NULL},
{XK_Delete,	E,	(FP)F_del_blanks,	  NULL, NULL},
{XK_Delete,	M|C|S,	(FP)F_file_delete,	  NULL, NULL},
#if defined(hpux)
{XK_DeleteChar,	0,	(FP)F_forw_del_char,	  NULL, NULL},
#endif
{XK_Down,	0,	(FP)F_forw_line,	  NULL, NULL},
{XK_Down,	C,	(FP)F_down_window,	  NULL, NULL},
{XK_Down,	S,	(FP)F_select_forw_line,	  NULL, NULL},
{XK_Down,	S|C,	(FP)F_all_win_down,	  NULL, NULL},
{XK_F34,	C,	(FP)F_down_window,	  NULL, NULL},
{XK_F34,	S,	(FP)F_select_forw_line,	  NULL, NULL},
{XK_F34,	S|C,	(FP)F_all_win_down,	  NULL, NULL},
{XK_KP_2,	C,	(FP)F_down_window,	  NULL, NULL},
{XK_KP_2,	S,	(FP)F_select_forw_line,	  NULL, NULL},
{XK_KP_2,	S|C,	(FP)F_all_win_down,	  NULL, NULL},
{XK_End,	0,	(FP)F_goto_eol,		  NULL, NULL},
{XK_End,	C,	(FP)F_goto_eob,		  NULL, NULL},
{XK_End,	S,	(FP)F_select_eol,	  NULL, NULL},
{XK_End,	S|C,	(FP)F_select_eob,	  NULL, NULL},
{XK_R13,	0,	(FP)F_goto_eol,		  NULL, NULL},
{XK_R13,	C,	(FP)F_goto_eob,		  NULL, NULL},
{XK_R13,	S,	(FP)F_select_eol,	  NULL, NULL},
{XK_R13,	S|C,	(FP)F_select_eob,	  NULL, NULL},
{XK_KP_1,	C,	(FP)F_goto_eob,		  NULL, NULL},
{XK_KP_1,	S,	(FP)F_select_eol,	  NULL, NULL},
{XK_KP_1,	S|C,	(FP)F_select_eob,	  NULL, NULL},
{XK_Execute,	0,	(FP)F_ins_nl,		  NULL, NULL},
{XK_F1,		0,	(FP)F_file_visit,	  NULL, NULL},
{XK_F1,		S,	(FP)F_view_file,	  NULL, NULL},
{XK_F1,		C,	(FP)F_insert_file,	  NULL, NULL},
{XK_F1,		C|S,	(FP)F_split_visit,	  NULL, NULL},
{XK_F10,	0,	(FP)F_copy_region,	  NULL, NULL},
{XK_F11,	0,	(FP)F_kill_region,	  NULL, NULL},
{XK_F11,	S,	(FP)F_file_write,	  NULL, NULL},
{XK_F12,	0,	(FP)F_yank,		  NULL, NULL},
{XK_F12,	S,	(FP)F_delete_curbuf,	  NULL, NULL},
{XK_F12,	C|S,	(FP)F_exit_flush_all,	  NULL, NULL},
{XK_F2,		0,	(FP)F_forw_window,	  NULL, NULL},
{XK_F2,		C,	(FP)F_split_window,	  NULL, NULL},
{XK_F2,		S,	(FP)F_only_window,	  NULL, NULL},
{XK_F3,		0,	(FP)F_goto_line,	  NULL, NULL},
{XK_F3,		S,	(FP)F_back_window,	  NULL, NULL},
{XK_F4,		0,	(FP)F_forw_buffer,	  NULL, NULL},
{XK_F4,		C,	(FP)F_help,		  NULL, NULL},
{XK_F4,		S,	(FP)F_execute_macro,	  NULL, NULL},
{XK_F5,		0,	(FP)F_indent,		  NULL, NULL},
{XK_F5,		S,	(FP)F_regindent,	  NULL, NULL},
{XK_F6,		0,	(FP)F_dedent,		  NULL, NULL},
{XK_F6,		S,	(FP)F_regdedent,	  NULL, NULL},
{XK_F7,		0,	(FP)F_query_replace,	  NULL, NULL},
{XK_F7,		C|S,	(FP)F_query_replace_all,  NULL, NULL},
{XK_F7,		E,	(FP)F_unmodify,		  NULL, NULL},
{XK_F7,		M|C|S,	(FP)F_file_reload,	  NULL, NULL},
{XK_F8,		0,	(FP)F_forw_search,	  NULL, NULL},
{XK_F8,		C,	(FP)F_search_again,	  NULL, NULL},
{XK_F8,		S,	(FP)F_back_search,	  NULL, NULL},
{XK_F8,		C|S,	(FP)F_forw_search_all,	  NULL, NULL},
{XK_F9,		0,	(FP)F_set_mark,		  NULL, NULL},
{XK_F9,		S,	(FP)F_kill_region,	  NULL, NULL},
{XK_Help,	0,	(FP)F_help,		  NULL, NULL},
{XK_Help,	C,	(FP)F_display_bindings,	  NULL, NULL},
{XK_Help,	S,	(FP)F_describe_key,	  NULL, NULL},
{XK_Help,	C|S,	(FP)F_display_commands,	  NULL, NULL},
{XK_Home,	0,	(FP)F_goto_bol,		  NULL, NULL},
{XK_Home,	S,	(FP)F_select_bol,	  NULL, NULL},
{XK_Home,	C,	(FP)F_goto_bob,		  NULL, NULL},
{XK_Home,	S|C,	(FP)F_select_bob,	  NULL, NULL},
{XK_F27,	0,	(FP)F_goto_bol,		  NULL, NULL},
{XK_F27,	C,	(FP)F_goto_bob,		  NULL, NULL},
{XK_F27,	S,	(FP)F_select_bol,	  NULL, NULL},
{XK_F27,	S|C,	(FP)F_select_bob,	  NULL, NULL},
{XK_KP_7,	C,	(FP)F_goto_bob,		  NULL, NULL},
{XK_KP_7,	S,	(FP)F_select_bol,	  NULL, NULL},
{XK_KP_7,	S|C,	(FP)F_select_bob,	  NULL, NULL},
{XK_Insert,	0,	(FP)F_ins_toggle,	  NULL, NULL},
{XK_Left,	0,	(FP)F_back_char,	  NULL, NULL},
{XK_Left,	C,	(FP)F_back_word,	  NULL, NULL},
{XK_Left,	S,	(FP)F_select_back_char,	  NULL, NULL},
{XK_Left,	S|C,	(FP)F_select_back_word,	  NULL, NULL},
{XK_Left,	M,	(FP)F_scroll_left,	  NULL, NULL},
{XK_F30,	C,	(FP)F_back_word,	  NULL, NULL},
{XK_F30,	S,	(FP)F_select_back_char,	  NULL, NULL},
{XK_F30,	S|C,	(FP)F_select_back_word,	  NULL, NULL},
{XK_KP_4,	C,	(FP)F_back_word,	  NULL, NULL},
{XK_KP_4,	S,	(FP)F_select_back_char,	  NULL, NULL},
{XK_KP_4,	S|C,	(FP)F_select_back_word,	  NULL, NULL},
{XK_Next,	0,	(FP)F_forw_page,	  NULL, NULL},
{XK_Next,	C,	(FP)F_goto_eob,		  NULL, NULL},
{XK_F35,	0,	(FP)F_forw_page,	  NULL, NULL},
{XK_F35,	C,	(FP)F_goto_eob,		  NULL, NULL},
{XK_Prior,	0,	(FP)F_back_page,	  NULL, NULL},
{XK_Prior,	C,	(FP)F_goto_bob,		  NULL, NULL},
{XK_F29,	0,	(FP)F_back_page,	  NULL, NULL},
{XK_F29,	C,	(FP)F_goto_bob,		  NULL, NULL},
{XK_Return,	0,	(FP)F_ins_nl,		  NULL, NULL},
{XK_Return,	C,	(FP)F_select_line,	  NULL, NULL},
{XK_Return,	M|S,	(FP)F_line_shell_cmd,	  NULL, NULL},
{XK_Return,	M,	(FP)F_execute_macro,	  NULL, NULL},
{XK_Right,	0,	(FP)F_forw_char,	  NULL, NULL},
{XK_Right,	C,	(FP)F_forw_word,	  NULL, NULL},
{XK_Right,	S,	(FP)F_select_forw_char,	  NULL, NULL},
{XK_Right,	S|C,	(FP)F_select_forw_word,	  NULL, NULL},
{XK_Right,	M,	(FP)F_scroll_right,	  NULL, NULL},
{XK_F32,	C,	(FP)F_forw_word,	  NULL, NULL},
{XK_F32,	S,	(FP)F_select_forw_char,	  NULL, NULL},
{XK_F32,	S|C,	(FP)F_select_forw_word,	  NULL, NULL},
{XK_KP_6,	C,	(FP)F_forw_word,	  NULL, NULL},
{XK_KP_6,	S,	(FP)F_select_forw_char,	  NULL, NULL},
{XK_KP_6,	S|C,	(FP)F_select_forw_word,	  NULL, NULL},
{XK_Tab,	0,	(FP)F_insert_tab,	  NULL, NULL},
{XK_Tab,	C,	(FP)F_realtab,		  NULL, NULL},
{XK_Tab,	E,	(FP)F_back_tab,		  NULL, NULL},
{XK_Tab,	C|S,	(FP)F_trim_line,	  NULL, NULL},
{XK_Tab,	M|C|S,	(FP)F_trim_buffer,	  NULL, NULL},
#if defined(hpux)
{XK_BackTab,	E,	(FP)F_back_tab,		  NULL, NULL},
#endif
{XK_Undo,	0,	(FP)F_default_back,	  NULL, NULL},
{XK_Up,		0,	(FP)F_back_line,	  NULL, NULL},
{XK_Up,		C,	(FP)F_up_window,	  NULL, NULL},
{XK_Up,		S,	(FP)F_select_back_line,	  NULL, NULL},
{XK_Up,		S|C,	(FP)F_all_win_up,	  NULL, NULL},
{XK_F28,	C,	(FP)F_up_window,	  NULL, NULL},
{XK_F28,	S,	(FP)F_select_back_line,	  NULL, NULL},
{XK_F28,	S|C,	(FP)F_all_win_up,	  NULL, NULL},
{XK_KP_8,	C,	(FP)F_up_window,	  NULL, NULL},
{XK_KP_8,	S,	(FP)F_select_back_line,	  NULL, NULL},
{XK_KP_8,	S|C,	(FP)F_all_win_up,	  NULL, NULL},

{KButton1,	0,	(FP)F_press_select,	  NULL, NULL},
{KButtonUp1,Button1Mask,(FP)F_release_select,	  NULL, NULL},
{KButton2,	0,	(FP)F_press_paste,	  NULL, NULL},
{KMotion,  Button1Mask, (FP)F_drag_button,	  NULL, NULL},

{KButton4,      0,      (FP)F_back_line6,         NULL, NULL},
{KButton5,      0,      (FP)F_forw_line6,         NULL, NULL},

{1,		0,	(FP)F_search_for_match,	  NULL, NULL},
};

#define NKEYS (sizeof(keys) / sizeof(KEY))


static int keyhash _ARGS1(KEY *, keyp)

{
    return (keyp->keysym ^ keyp->modifier) & 0xff;
}


KEY *emxgetbinding _ARGS1(KEY *, keyp)

{
    int	    hash;
    KEY	    *kp;

    /* Check special typing key bindings first */
    if (keyp->keysym <= 0xff && keyp->modifier == 0)
	{
	/* Check for automatch */
	if (g_modes.automatch && g_matchp &&
	    (keyp->keysym == g_matchp->automatchchar ||
	     keyp->keysym == g_matchp->automatchchar2))
	    {
	    g_matchkeyp->keysym = keyp->keysym;
	    return g_matchkeyp;
	    }

	g_keylistp->keysym = keyp->keysym;
	return g_keylistp;
	}

    /* Look up the key in the hash table */
    hash = keyhash(keyp);
    if (!(kp = g_keyhashtable[hash]))
	{
#ifdef SHOWNAMES
	keyname(keyp, buf);
	puts(buf);
#endif
	return keyp;
	}

    do
	{
	if (kp->keysym == keyp->keysym && kp->modifier == keyp->modifier)
	    {
#ifdef SHOWNAMES
	    keyname(keyp, buf);
	    puts(buf);
#endif
	    return kp;
	    }

	kp = kp->hashp;
	}
	while (kp);

#ifdef SHOWNAMES
	keyname(keyp, buf);
	puts(buf);
#endif
    return keyp;
}


/* This function waits for and 'returns' a single combined keystroke */

static int emxonekeyhandler _ARGS1(KEY *, keyp)

{
    /* Exit the state, and call the function */
    g_keyp = keyp;
    emxstackpop();
    return (*g_onefunc)(TRUE);
}


int emxonekey _ARGS1(FUNC, funcp)

{
    /* Initialize state variables */
    g_onefunc = funcp;

    /* Enter a new state */
    emxstackpush(emxonekeyhandler);
    return PARTIAL;
}


/* Transform a key code into a name */

void EmxKeyString _ARGS3(int, keysym, int, modifier, char *, bufp)

{
    unsigned long   value;
    char	    *ptr;
    int		    i;

    /* C-X prefix */
    if (modifier & KCtlxMask)
	{
	*bufp++ = '^';
	*bufp++ = 'X';
	*bufp++ = '-';
	}

    /* Escape prefix */
    if (modifier & KEscapeMask)
	{
	*bufp++ = 'E';
	*bufp++ = '-';
	}

    /* Mod5 prefix */
    if (modifier & Mod5Mask)
	{
	*bufp++ = 'M';
	*bufp++ = '5';
	*bufp++ = '-';
	}

    /* Mod4 prefix */
    if (modifier & Mod4Mask)
	{
	*bufp++ = 'M';
	*bufp++ = '4';
	*bufp++ = '-';
	}

    /* Mod3 prefix */
    if (modifier & Mod3Mask)
	{
	*bufp++ = 'M';
	*bufp++ = '3';
	*bufp++ = '-';
	}

    /* Mod2 prefix */
    if (modifier & Mod2Mask)
	{
	*bufp++ = 'M';
	*bufp++ = '2';
	*bufp++ = '-';
	}

    /* Mod1 prefix */
    if (modifier & Mod1Mask)
	{
	*bufp++ = 'M';
	*bufp++ = '1';
	*bufp++ = '-';
	}

    /* Ctrl prefix */
    if (modifier & ControlMask)
	{
	*bufp++ = '^';
	*bufp++ = '-';
	}

    /* Shift prefix */
    if (modifier & ShiftMask)
	{
	*bufp++ = 'S';
	*bufp++ = '-';
	}

    /* Watch for special values */
    if (keysym == 0)
	strcpy(bufp, "<No Key>");

    else if (keysym == 0x20)
	strcpy(bufp, "space");

    else
	{
	/* Return string, or build one */
	ptr = XKeysymToString(keysym);
	if (ptr)
	    strcpy(bufp, ptr);
	else
	    {
	    value = keysym;
	    for (i = 0; i < 4; i++)
		{
		*bufp++ = emxhex[(value >> 28) & 0x0f];
		*bufp++ = emxhex[(value >> 24) & 0x0f];
		value <<= 8;
		}

	    *bufp = 0;
	    }
	}

#if 0
    else if (keysym > 0x20 && keysym <= 0xff)
	{
	*bufp++ = keysym;
	*bufp = 0;
	}

    else
	{
	for (strp = g_keystrings; strp->keysym && strp->keysym != keysym;
	     strp++)
	    ;

	/* Return string, or build one */
	if (strp->keysym)
	    strcpy(bufp, strp->namep);
	else
	    {
	    value = keysym;
	    for (i = 0; i < 4; i++)
		{
		*bufp++ = emxhex[(value >> 28) & 0x0f];
		*bufp++ = emxhex[(value >> 24) & 0x0f];
		value <<= 8;
		}

	    *bufp = 0;
	    }

	}
#endif
}


/* Transform a key code into a name */

static void keyname _ARGS2(KEY *, kp, char *, bufp)

{
    EmxKeyString(kp->keysym, kp->modifier, bufp);
}


/* Function table lookup. Return a pointer to the FUNCTION node, or NULL if
   the symbol is not found. */

static FUNCTION *funclookup _ARGS1(char *, namep)

{
    FUNCTION	*fp;

    /* Search sequentially through the alphabetic symbol list */
    for (fp = g_flistp; fp && strcmp(namep, fp->namep) != 0; fp = fp->nextp)
	;

    return fp;
}


/* Bind a given key to a named function */

static void bindtofunc _ARGS3(KEY *, keyp, FUNCTION *, funcp, int, original)

{
    KEYBINDING	*bindp;
    KEYBINDING	*bp;

    keyp->funcp = funcp;

    /* Tell the function it is bound to this key */
    bindp = (KEYBINDING *) malloc(sizeof(KEYBINDING));
    if (!funcp->bindingp)
	funcp->bindingp = bindp;
    else
	{
	for (bp = funcp->bindingp; bp->nextp; bp = bp->nextp)
	    ;

	bp->nextp = bindp;
	}

    bindp->keyp = keyp;
    bindp->nextp = NULL;
    bindp->original = original;
}


/* Remove a key's binding */

static void unbind _ARGS1(KEY *, keyp)

{
    KEYBINDING	*pp;
    KEYBINDING	*bp;
    FUNCTION	*funcp;

    /* Special case for unnamed macros */
    funcp = keyp->funcp;
    if ((funcp->type & SYMMACRO) && (funcp->namep == NULL))
	{
	/* Free the macro and the function */
	FREE(funcp->procp);
	FREE(funcp);
	}

    else
	{
	pp = NULL;
	for (bp = funcp->bindingp; bp && (bp->keyp != keyp); bp = bp->nextp)
	    pp = bp;

	if (bp)
	    {
	    if (pp)
		pp->nextp = bp->nextp;
	    else
		funcp->bindingp = bp->nextp;

	    FREE(bp);
	    }
	}

    /* Detach the function from the key, unless it's a special */
    keyp->funcp = NULL;
}


/* Add a new key entry to the key hash table */

static void addkeytohash _ARGS1(KEY *, keyp)

{
    int	    hash;

    /* Hash this key into the fast key lookup table */
    hash = keyhash(keyp);
    keyp->hashp = g_keyhashtable[hash];
    g_keyhashtable[hash] = keyp;
}


static void emxkeylistinit _ARGS0()


{
    KEY		*endkp;
    KEY		*kp;
    KEY		*newkp;
    KEY		*pkp;
    FUNCTION	*fp;

    /* Clear out the hash table */
    memset(g_keyhashtable, 0, sizeof(g_keyhashtable));

    /* Bind the typing keys with a single entry */
    fp = g_fselfinsertp;

    pkp = kp = g_keylistp = (KEY *) malloc(sizeof(KEY));
    kp->keysym = ' ';
    kp->modifier = 0;

    /* Bind it to the function, but don't bother with the reverse */
    kp->funcp = fp;

    /* Add it to the hash table */
    addkeytohash(kp);

    endkp = keys + NKEYS;

    /* Loop through the default bindings, linking them to functions */
    for (kp = keys; kp < endkp; kp++)
	{
	newkp = (KEY *) malloc(sizeof(KEY));
	memcpy(newkp, kp, sizeof(KEY));

	pkp->orderp = newkp;

	/* Find the function it is bound to */
	for (fp = g_flistp; fp && (((char *) newkp->funcp) != fp->namep);
	     fp = fp->nextp)
	    ;

	/* Bind it to the function */
	bindtofunc(newkp, fp, TRUE);

	/* Add it to the hash table */
	addkeytohash(newkp);
	pkp = newkp;

	/* A special */
	if (fp->procp == emxsearchformatch)
	    g_matchkeyp = newkp;
	}

    g_keylastp = pkp;
}


/* Set up the function table and bind all the default keys */

void emxkeymapinit _ARGS0()


{
    FUNCTION	*endfp;
    FUNCTION	*fp;
    FUNCTION	*newfp;
    FUNCTION	*pfp;

    /* Loop through the functions, setting up pointers */
    endfp = functions + NFUNCS;
    pfp = NULL;
    for (fp = functions; fp < endfp; fp++)
	{
	newfp = (FUNCTION *) malloc(sizeof(FUNCTION));
	memcpy(newfp, fp, sizeof(FUNCTION));

	if (fp == functions)
	    g_flistp = newfp;

	if ((newfp->prevp = pfp))
	    pfp->nextp = newfp;

	pfp = newfp;

	/* Save interesting pointers */
	if (newfp->procp == emxabort)
	    g_fabortp = newfp;
	else if (newfp->procp == emxselfinsert)
	    g_fselfinsertp = newfp;
	else if (newfp->procp == emxbindtokey)
	    g_fbindtokeyp = newfp;
	}

    pfp->nextp = NULL;

    emxkeylistinit();
}


/* This function modifies the keyboard binding table, by adjusting the entries
   in the big "bindings" array. Most of the grief deals with the prompting for
   additional arguments. */

static int bindkey _ARGS1(KEY *, kp)

{
    KEY *newp;

    if (kp->keysym == 0)
	{
	emxmsgprint(g_errbindnull);
	return FALSE;
	}

    if (kp->funcp == g_fselfinsertp)
	{
	emxmsgprint(g_errbindtyping);
	return FALSE;
	}

    /* If bound, unbind first */
    if (kp->funcp)
	unbind(kp);

    /* Not bound - allocate space and add to the hash and order lists */
    else
	{
	newp = (KEY *) malloc(sizeof(KEY));
	newp->keysym = kp->keysym;
	newp->modifier = kp->modifier;
	addkeytohash(newp);

	/* Add to the end of the order list */
	g_keylastp->orderp = newp;
	newp->orderp = NULL;
	g_keylastp = newp;
	kp = newp;
	}

    /* Bind to the new function */
    bindtofunc(kp, g_funcp, FALSE);
    return TRUE;
}


static void execbuf _ARGS2(BUFFER *, bp, int, delete)

{
    g_execline = bp->linep->nextp;
    g_executing = TRUE;
    g_execview = bp->flag & BFVIEW;
    bp->flag |= BFVIEW;		  /* Make buffer read-only */
    g_execbuf = bp;
    g_execpos = 0;
    g_tokp = g_tokbuf;
    *g_tokp = 0;
    g_tempbuf = delete;
}


static void execstring _ARGS2(const char *, str, int, delete)

{
    if (!str || str[0] == 0)
	return;

    g_executing = TRUE;
    g_execstr = str;
    g_tokp = g_tokbuf;
    *g_tokp = 0;
    g_tempbuf = delete;
}


static void endexec _ARGS0()

{
    if (g_execbuf)
	{
	/* Reset buffer view state */
	g_execbuf->flag = (g_execbuf->flag & ~BFVIEW) | g_execview;
	if (g_tempbuf)
	    emxdeletebuffer(g_execbuf);
	else
	    emxinfoupdate();
	}

    else if (g_tempbuf)
	FREE(g_execstr);

    g_executing = FALSE;
    g_execbuf = NULL;
    g_execstr = NULL;

    if (!g_initialload && !g_epresent)
	emxmsgprint("[Executed]");
}


static int abortexec _ARGS1(const char *, str)

{
    endexec();
    emxmsgprintstr("[Execute: %s]", str);
    return FALSE;
}


static COMPLETION(emxbindtokeycomplete)

{
    char    kname[MAXCMD];

    if (!g_initialload)
	{
	keyname(g_keyp, kname);		    /* Display keyname */
	emxmsgput(kname);
	}

    emxpromptoff();
    return bindkey(g_keyp);
}


static COMPLETION(emxbindtokeypartial)

{
    if (status != TRUE)
	return status;

   if ((g_funcp = funclookup(g_funcname)) == NULL)
	{
	emxmsgprint("[Unknown function for binding]");
	return FALSE;
	}

    emxprompton();
    emxmsgput(" to key: ");
    return emxonekey(emxbindtokeycomplete);
}


COMMAND(emxbindtokey)

{
    return emxmsgread("Bind function: ", g_funcname, sizeof(g_funcname),
		   NEW|AUTO, emxbindtokeypartial);
}


/* Bind the current (unnamed) keyboard macro to a key */

static COMPLETION(emxbindmacrocomplete)

{
    KEY	*kp;
    KEY	*newp;
    FUNCTION	*fp;
    int			maclen, i;
    char		xname[MAXCMD];

    kp = g_keyp;
    keyname(kp, xname);			/* Display keyname.	*/
    emxmsgput(xname);
    emxpromptoff();

    if (kp->keysym == 0)
	{
	emxmsgprint(g_errbindnull);
	return FALSE;
	}

    if (kp->funcp == g_fselfinsertp)
	{
	emxmsgprint(g_errbindtyping);
	return FALSE;
	}

    /* If bound, unbind first */
    if (kp->funcp)
	unbind(kp);

    /* Not bound - allocate space and add to the hash and order lists */
    else
	{
	newp = (KEY *) malloc(sizeof(KEY));
	newp->keysym = kp->keysym;
	newp->modifier = kp->modifier;
	addkeytohash(newp);

	/* Add to the end of the order list */
	g_keylastp->orderp = newp;
	newp->orderp = NULL;
	g_keylastp = newp;
	kp = newp;
	}

    for (maclen = 0; g_macro[maclen]; maclen += 2)
	;

    maclen++;

    /* Create a dummy function entry for this macro */
    if ((fp = (FUNCTION *) malloc(sizeof(FUNCTION))) == 0)
	{
	emxallocerr(sizeof(FUNCTION));
	return 0;
	}

    fp->namep = NULL;
    fp->type = SYMMACRO;
    fp->bindingp = NULL;

    i = sizeof(unsigned long) * maclen;
    if ((fp->procp = (FUNC) malloc(i)) == 0)
	{
	FREE(fp);
	return emxallocerr(i);
	}

    memcpy((char *)fp->procp, (char *)g_macro, maclen * sizeof(unsigned long));
    kp->funcp = fp;
    return TRUE;
}


COMMAND(emxbindmacro)

{
    if (g_macro == NULL)
	{
	emxmsgprint(g_msgnomacro);
	return FALSE;
	}

    emxprompton();
    emxmsgprint(": bind macro to key ");
    return emxonekey(emxbindmacrocomplete);
}


/* Remove the binding from a currently bound key. */

static COMPLETION(emxunbindkeycomplete)

{
    KEY    *kp;
    char	    xname[MAXCMD];

    kp = g_keyp;
    keyname(kp, xname);			     /* Display keyname.     */
    emxmsgput(xname);
    emxpromptoff();

    if (kp->funcp == NULL)
	emxmsgprint("[Key not bound]");
    else if (kp->funcp == g_fselfinsertp)
	emxmsgprint("[Cannot unbind typing characters]");
    else
	unbind(kp);

    return TRUE;
}


COMMAND(emxunbindkey)

{
    emxprompton();
    emxmsgprint(": unbind-key ");
    return emxonekey(emxunbindkeycomplete);
}


/* Extended command. Call the message line routine to read in the command name
   and apply autocompletion to it. When it comes back, look the name up in the
   symbol table and run the command if it is found and has the right type.
   Print an error if there is anything wrong. */

COMPLETION(emxextendedcommandcomplete)

{
    FUNCTION	*fp;

    if (status != TRUE)
	return status;

    /* Special case bind-to-key for speed */
    if (strcmp(g_funcname, F_bind_to_key) == 0)
	fp = g_fbindtokeyp;
    else
	fp = funclookup(g_funcname);

    if (fp)
	return emxexecfunc(fp, g_flag, g_num, &g_keynull);

    emxmsgprint("[Unknown command]");
    return ABORT;
}


COMMAND(emxextendedcommand)

{
    g_flag = f;
    g_num = n;
    return emxmsgread(": ", g_funcname, sizeof(g_funcname), NEW|AUTO,
		 emxextendedcommandcomplete);
}


/* Read a key from the keyboard, and look it up in the binding table. Display
   the name of the function currently bound to the key. */

static COMPLETION(emxdescribekeycomplete)

{
    KEY    *kp;
    char	    xname[MAXCMD];
    char	    buf[256];

    kp = g_keyp;
    emxpromptoff();
    keyname(kp, xname);
    if (kp->funcp == NULL)
	emxmsgprintstr("[%s is unbound]", xname);
    else
	{
	sprintf(buf, "[%s is bound to %s]", xname, kp->funcp->namep);
	emxmsgprint(buf);
	}

    return TRUE;
}


COMMAND(emxdescribekey)

{
    emxmsgprint(": describe-key ");
    emxprompton();
    return emxonekey(emxdescribekeycomplete);
}


/* This function creates a table, listing all of the command keys and their
   current bindings, and stores the table in the standard pop-up buffer (the
   one used by the directory list command, the buffer list command, etc.). This
   lets EMX produce it's own wall chart. The bindings to "self-insert" are only
   displayed if there is an argument. */

COMMAND(emxdisplaybindings)

{
    int    i;
    KEY    *kp;
    char   *cp1;
    int		    tabs;
    char	buf[64];

    emxbclear(g_blistp);
    strcpy(g_blistp->bufname, g_strkeylist);

    /* Give the key binding file a header */
    if (emxaddline(g_blistp, "Key Name\t\tCommand Name") == FALSE ||
	emxaddline(g_blistp, "--------\t\t------------") == FALSE)
	return FALSE;

#define COLUMN	24

    /* Display all bound keys */
    for (i = 0; i < MODIFIERS; i++)
	{
	for (kp = g_keylistp; kp; kp = kp->orderp)
	    {
	    /* Write only those with this modifier */
	    if (kp->modifier == modifiers[i] &&
		(f || kp->funcp != g_fselfinsertp))
		{
		keyname(kp, buf);
		cp1 = buf + strlen(buf);	/* Find end */
		tabs = (COLUMN - (cp1 - buf) - 1) / 8 + 1;
		while (tabs--)
		    *cp1++ = '\t';

		if (kp->funcp->namep)
		    {
		    strcpy(cp1, kp->funcp->namep);
		    if (kp->funcp->type & SYMMACRO)
			strcat(cp1, " (Macro)");
		    }

		else
		    strcpy(cp1, g_strunnamed);

		if (!emxaddline(g_blistp, buf))
		    return FALSE;
		}

	    }

	}

    /* Now catch all the dregs */
    for (kp = g_keylistp; kp; kp = kp->orderp)
	{
	/* Write only if none of the above */
	for (i = 0; i < MODIFIERS; i++)
	    if (kp->modifier == modifiers[i])
		break;

	if (i == MODIFIERS &&
	    (f || kp->funcp != g_fselfinsertp))
	    {
	    keyname(kp, buf);
	    cp1 = buf + strlen(buf);	    /* Find end */
	    tabs = (COLUMN - (cp1 - buf) - 1) / 8 + 1;
	    while (tabs--)
		*cp1++ = '\t';

	    if (kp->funcp->namep)
		{
		strcpy(cp1, kp->funcp->namep);
		if (kp->funcp->type & SYMMACRO)
		    strcat(cp1, " (Macro)");
		}

	    else
		strcpy(cp1, g_strunnamed);

	    if (!emxaddline(g_blistp, buf))
		return FALSE;
	    }

	}

    return emxpopblist(FALSE);
}


/* This function creates a table, listing all of the command keys and their
   current bindings, and stores the table in the standard pop-up buffer (the
   one used by the directory list command, the buffer list command, etc.). This
   lets EMX produce its own wall chart. The bindings to "self-insert" are only
   displayed if there is an argument. */

COMMAND(emxdisplaycommands)

{
    FUNCTION	*fp;
    int		len;
    KEYBINDING	*bp;
    int		i;
    int		tabs;
    char	*ptr;
    char	buf[256];

    emxbclear(g_blistp);
    strcpy(g_blistp->bufname, g_strfunclist);

#define TYPE	24
#define BINDING 32

    /* Give the function list a header */
    if (emxaddline(g_blistp, "Command Name\t\tType\tBindings") == FALSE ||
	emxaddline(g_blistp, "------------\t\t----\t--------") == FALSE)
	return FALSE;

    /* Display all functions and macros */
    for (fp = g_flistp; fp; fp = fp->nextp)
	{
	len = strlen(fp->namep);
	strcpy(buf, fp->namep);
	ptr = buf + len;
	tabs = (TYPE - len - 1) / 8 + 1;
	while (tabs--)
	    *ptr++ = '\t';

	if (fp->type & SYMMACRO)
	    strcpy(ptr, "Macro");
	else
	    strcpy(ptr, "Func");

	if (fp->bindingp)
	    {
	    ptr = buf + strlen(buf);

	    /* Print all bindings in preferred order */
	    for (i = 0; i < MODIFIERS; i++)
		{
		for (bp = fp->bindingp; bp; bp = bp->nextp)
		    {
		    if (bp->keyp->modifier == modifiers[i])
			{
			*ptr++ = '\t';
			keyname(bp->keyp, ptr);
			ptr += strlen(ptr);
			}

		    }

		}

	    /* Catch all the rest */
	    for (bp = fp->bindingp; bp; bp = bp->nextp)
		{
		/* Make sure it wasn't shown above */
		for (i = 0; i < MODIFIERS; i++)
		    if (bp->keyp->modifier == modifiers[i])
			break;

		if (i == MODIFIERS)
		    {
		    *ptr++ = '\t';
		    keyname(bp->keyp, ptr);
		    ptr += strlen(ptr);
		    }

		}

	    *ptr = 0;
	    }

	else if (fp == g_fselfinsertp)
	    strcat(buf, "\t<all characters>");

	if (!emxaddline(g_blistp, buf))
	    return FALSE;
	}

    return emxpopblist(FALSE);
}


/* Execute the contents of the current buffer */

static COMPLETION(emxexecutebuffercomplete)

{
    BUFFER  *bp;

    if (status == ABORT)
	return status;

    if (g_cmdbuf[0] == 0)
	bp = g_curbp;

    /* Must already exist */
    else if ((bp = emxbfind(g_cmdbuf, FALSE)) == NULL)
	{
	emxmsgnotfound(g_cmdbuf);
	return FALSE;
	}

    execbuf(bp, FALSE);
    return TRUE;
}


COMMAND(emxexecutebuffer)

{
    if (g_macroinp || g_macrooutp)
	{
	emxmsgprint(g_msgnotnow);
	return FALSE;
	}

    if (g_execbuf) {
	emxmsgprint("[Already executing a buffer]");
	return FALSE;
	}

    /* Prompt for a buffer name */
    return emxmsgreply("Execute buffer: ", g_cmdbuf, sizeof(g_cmdbuf),
		    emxexecutebuffercomplete);
}


/* Type a macro as text */

static COMPLETION(emxtypemacrocomplete)

{
    unsigned long   *macrop;
    FUNCTION	    *fp;

    char	    *p1;

    char	    string[256];

    if (status == ABORT)
	return status;

    /* Find the macro */
    if (g_funcname[0] == 0)
	{
	if (!(macrop = g_macro))
	    {
	    emxmsgprint(g_msgnomacro);
	    return FALSE;
	    }
	}

    else
	{
	if (!(fp = funclookup(g_funcname)))
	    {
	    emxmsgnotfound(g_funcname);
	    return FALSE;
	    }

	macrop = (unsigned long *) fp->procp;
	}

    /* Decode the keys into strings and insert into the buffer */
    while (*macrop)
	{
	g_keycurrent.keysym = *macrop++;
	g_keycurrent.modifier = *macrop++;
	keyname(&g_keycurrent, string);

	for (p1 = string; *p1; p1++)
	    {
	    if (*p1 == '"')
		emxinsertchar(1, '\\');

	    emxinsertchar(1, *p1);
	    emxupdate();
	    }

	/* Insert a space */
	emxinsertchar(1, ' ');
	emxupdate();
	}

    /* Insert a newline */
    emxlnewline();
    return TRUE;
}


COMMAND(emxtypemacro)

{
    if (g_macroinp || g_macrooutp)
	{
	emxmsgprint(g_msgnotnow);
	return FALSE;
	}

    /* Prompt for a macro name */
    return emxmsgread("Type macro: ", g_funcname, sizeof(g_funcname),
		   NEW|AUTO, emxtypemacrocomplete);
}


/* Read in a token from the execution buffer. Returns 0 if EOF, 1 if normal
   word read, 2 if quoted string read. */

static int gettok _ARGS0()

{
    char    *tp;
    char    *endp;
    char    c;
    const char *cp;
    int	    skipping;
    int	    quoting;
    int	    escape;

    tp = g_tokp = g_tokbuf;
    endp = tp + sizeof(g_tokbuf) - 2;
    *tp = 0;

    skipping = TRUE;
    quoting = FALSE;
    escape = FALSE;

    while (tp < endp)
	{
	/* String? */
	if (cp = g_execstr)
	    {
	    if ((c = *cp++))
		g_execstr = cp;
	    }

	/* Buffer */
	else
	    {
	    if (g_execline == g_execbuf->linep)
		c = 0;

	    else if (g_execpos == g_execline->used)
		{
		g_execline = g_execline->nextp;
		g_execpos = 0;
		c = '\n';
		}

	    else
		c = g_execline->text[g_execpos++];
	    }

	/* No more characters? */
	if (!c)
	    break;

	/* Got a character - do what with it? */
	if (escape)
	    {
	    *tp++ = c;
	    escape = FALSE;
	    continue;
	    }

	if (quoting)
	    {
	    if (c == '\\')
		{
		*tp++ = c;
		escape = TRUE;
		continue;
		}

	    if (c == '"')
		break;

	    *tp++ = c;
	    continue;
	    }

	if (skipping)
	    {
	    if (ISSPACE(c))
		continue;

	    skipping = FALSE;
	    }

	else if (ISSPACE(c))
	    break;

	if (c == '\\')
	    {
	    escape = TRUE;
	    continue;
	    }

	if (c == '"')
	    {
	    quoting = TRUE;
	    continue;
	    }

	*tp++ = c;
	}

    /* No more characters */
    if (tp == endp)
	return abortexec("token too long");

    /* Terminate the token and return */
    *tp++ = 0;
    *tp = 0;

    if (quoting)
	{
	if (c != '"')
	    return abortexec("unpaired quote");

	return 2;
	}

    if (skipping)
	{
	endexec();
	return 0;
	}

    return 1;
}


/* Get a single logical key from the execution buffer. This handles
   interpretation of words and strings. Returns -1 if no execution buffer. */

KEY *emxgetbufkey _ARGS0()

{
    int		i;
    const char	*namep;
    char *tokp, ch1, c1, c2;
    KEY		*keyp;
    KEYSTRING	*strp;

    if (!g_executing)
	return NULL;

    /* See if we're working on a normal word. End of word is flagged by a 0. */
    keyp = &g_keycurrent;
    keyp->modifier = 0;
    if ((keyp->keysym = *g_tokp++))
	return keyp;

    /* End of buffer? */
    if ((i = gettok()) == 0)
	return NULL;

    /* Quoted string - return characters for a while */
    if (i == 2)
	{
	keyp->keysym = *g_tokp++;
	return keyp;
	}

    /* A genuine token - see if it is a command */

    /* Check for special token: a solitary '|' means 'Return' */
    if (g_tokbuf[0] == '|' && g_tokbuf[1] == 0)
	{
	g_tokp++;
	keyp->keysym = XK_Return;
	return keyp;
	}

    /* Assume the word is a single key descriptor and pick out the prefixes */
    while (TRUE)
	{
	tokp = g_tokp;
	if (tokp[1] == '-')
	    {
	    if (tokp[0] == 'E')
		keyp->modifier |= KEscapeMask;

	    else if (tokp[0] == 'S')
		keyp->modifier |= ShiftMask;

	    else if (tokp[0] == '^')
		keyp->modifier |= ControlMask;

	    else
		break;

	    g_tokp += 2;
	    }

	else if (tokp[2] == '-')
	    {
	    if (tokp[0] == '^' && tokp[1] == 'X')
		keyp->modifier |=KCtlxMask;

	    else if (tokp[0] == 'M')
		{
		if (tokp[1] == '1')
		    keyp->modifier |= Mod1Mask;

		else if (tokp[1] == '2')
		    keyp->modifier |= Mod2Mask;

		else if (tokp[1] == '3')
		    keyp->modifier |= Mod3Mask;

		else if (tokp[1] == '4')
		    keyp->modifier |= Mod4Mask;

		else if (tokp[1] == '5')
		    keyp->modifier |= Mod5Mask;

		else
		    break;
		}

	    g_tokp += 3;
	    }

	else
	    break;
	}

    /* If there is actually only one character left after the prefixes then it
       really was a single-key descriptor, so return it. If we return, g_tokp
       is left pointing to the end of the word so the next call will need to
       get a new word. */

    if (*g_tokp && *(g_tokp + 1) == 0)
	{
	keyp->keysym = *g_tokp++;
	return keyp;
	}

    /* See if the remainder is a keysym name. Ignore case. */
    c1 = c2 = *tokp;
    if (ISLOWER(c2))
	c2 = TOUPPER(c2);
    else if (ISUPPER(c2))
	c2 = TOLOWER(c2);

    MARK(findkeyname);
    for (strp = g_keystrings; strp->keysym; strp++)
	{
	namep = strp->namep;
	if (*namep != c1 && *namep != c2)
	    continue;

	namep++;
	for (tokp = g_tokp + 1; (ch1 = *namep); namep++, tokp++)
	    {
	    if (ch1 != *tokp)
		{
		if (ISLOWER(ch1))
		    ch1 = TOUPPER(ch1);
		else if (ISUPPER(ch1))
		    ch1 = TOLOWER(ch1);

		if (ch1 != *tokp)
		    break;
		}
	    }

	if (*namep == *tokp)
	    {
	    g_tokp = tokp;
	    keyp->keysym = strp->keysym;
	    return keyp;
	    }

	}

    MARK(gotbufchar);
    /* A normal word - return characters for a while */
    g_tokp = g_tokbuf;
    keyp->keysym = *g_tokp++;
    return keyp;
}


/* Read a file into a buffer, execute the contents of the file and then
   kill the buffer. */

static int loadexecfile _ARGS2(char *, nm, int, holler)

{
    BUFFER *bp;    /* buffer to place file to exeute */
    BUFFER *cb;    /* temp to hold current buf while we read */
    int    status; /* results of various calls */
    int		    theAccess;

    if (g_execbuf) {
	emxmsgprint("[Already executing a buffer]");
	return FALSE;
	}

    if (nm == NULL)
	return FALSE;

    if ((bp = emxbfind("#e_file#", TRUE)) == NULL) /* get the needed buffer */
	return FALSE;

    emxbclear(bp);
    cb = g_curbp;		    /* save the old buffer */
    g_curbp = bp;		    /* make this one current */

    /* and try to read in the file to execute */
    emxexpandname(nm);	 /* shell vars */
    theAccess = READ;
    if ((status = emxffropen(nm, &theAccess)) == FIOSUC &&
	emxreadin(nm, theAccess, TRUE))
	{
	/* Execute it */
	g_curbp = cb;
	execbuf(bp, TRUE);
	return TRUE;
	}

    else if (status == FIOFNF && holler)
	emxmsgnotfound(nm);

    g_curbp = cb;
    emxdeletebuffer(bp);
    return FALSE;
}


static COMPLETION(emxexecutefilecomplete)

{
    if (status != TRUE)
	return status;

    return loadexecfile(g_filename, TRUE);
}


COMMAND(emxexecutefile)

{
    if (g_macroinp || g_macrooutp)
	{
	emxmsgprint(g_msgnotnow);
	return FALSE;
	}

    return emxmsgreply("Execute file: ", g_filename, MAXFILE, emxexecutefilecomplete);
}


/* Play a named file */

int emxplayfile _ARGS2(char *, namep, int, holler)

{
    if (loadexecfile(namep, holler) != TRUE)
	return FALSE;

    /* Enter the private key processing loop, and quit when done */
    g_keyp = NULL;
    emxkeyloop();
    return TRUE;
}


/* Play a string */

int emxplaystring _ARGS1(const char *, str)

{
    /* Already reading a string? */
    if (g_execstr) {
	emxmsgprint("[Already executing a string]");
	return FALSE;
	}

    /* Start reading the string */
    execstring(str, FALSE);

    /* Enter the private key processing loop, and quit when done */
    g_keyp = NULL;
    emxkeyloop();
    return TRUE;
}


/* Begin a keyboard macro.  Error if not at the top level in keyboard
   processing. Set up variables and return. */

COMMAND(emxstartmacro)

{
    if (g_macroinp || g_macrooutp)
	{
	emxmsgprint(g_msgnotnow);
	return FALSE;
	}

    emxmsgprint("[Start macro]");
    if (g_macro)
	FREE(g_macro);

    g_macroinp = g_macro = (unsigned long *) malloc(MAXMACRO *
						    sizeof(unsigned long));
    *g_macro = 0;
    return TRUE;
}


/* End keyboard macro. Check for the same limit conditions as the above
   routine. Set up the variables and return to the caller. */

COMMAND(emxendmacro)

{
    if (g_macrooutp)	      /* We're seeing it on playback */
	return TRUE;

    if (g_macroinp == NULL)
	{
	emxmsgprint("[No macro in progress]");
	return FALSE;
	}

    emxmsgprint("[End macro]");
    *g_savemacroinp = 0;
    if (g_macro[0] == 0)
	{
	FREE(g_macro);
	g_macro = NULL;
	}

    g_macroinp = NULL;
    return TRUE;
}


/* Execute a macro. The command argument is the number of times to loop. Quit
   as soon as a command gets an error. Return TRUE if all ok, else FALSE. */

int emxplaymacro _ARGS2(unsigned long *, macrop, int, n)

{
    if (g_macroinp || g_macrooutp)
	{
	emxmsgprint(g_msgnotnow);
	return FALSE;
	}

    if (n > 0)
	{
	/* Set the globals and go */
	g_macroplay = macrop;
	g_macrooutp = macrop;
	g_macroplays = n;
	}

    return TRUE;
}


/* Execute the unnamed macro */

COMMAND(emxexecutemacro)

{
    return emxplaymacro(g_macro, n);
}


/* Locate a macro with the specified name, or create one. */

static FUNCTION *findmacro _ARGS1(char *, mname)

{
    FUNCTION	*fp;
    FUNCTION	*sortp;
    int		len;
    char	*cp1;
    char	*cp2;

    if ((fp = funclookup(mname)))
	{
	if (fp->type & SYMFUNC)
	    {
	    emxmsgprint("[Name in use by existing function]");
	    return NULL;
	    }

	/* Free the key list from the old definition */
	FREE(fp->procp);
	fp->procp = (FUNC) NULL;
	}

    /* Create a new entry and chain it in */
    else
	{
	if ((fp = (FUNCTION *) malloc(sizeof(FUNCTION))) == 0)
	    {
	    emxallocerr(sizeof(FUNCTION));
	    return NULL;
	    }

	len = strlen(mname) + 1;
	if ((fp->namep = (char *) malloc(len)) == 0)
	    {
	    FREE(fp);
	    emxallocerr(len);
	    return NULL;
	    }

	/* Make the macro name lower-case */
	cp1 = mname;
	cp2 = fp->namep;
	while (*cp1)
	    *cp2++ = ISUPPER(*cp1) ? TOLOWER(*cp1++) : *cp1++;

	*cp2++ = 0;

	fp->type = SYMMACRO;
	fp->procp = (FUNC) NULL;
	fp->bindingp = NULL;

	/* Sort it into the list */
	for (sortp = g_flistp; sortp && strcmp(fp->namep, sortp->namep) > 0;
	     sortp = sortp->nextp)
	    ;

	if (sortp)
	    {
	    if (sortp->prevp)
		{
		sortp->prevp->nextp = fp;
		fp->prevp = sortp->prevp;
		}

	    else
		{
		g_flistp = fp;
		fp->prevp = NULL;
		}

	    sortp->prevp = fp;
	    }

	fp->nextp = sortp;
	}

    return fp;
}


/* Pull a named macro out of the symbol list */

static void removemacro _ARGS1(FUNCTION *, fp)

{
    if (fp->prevp)
	fp->prevp->nextp = fp->nextp;
    else
	g_flistp = fp->nextp;

    if (fp->nextp)
	fp->nextp->prevp = fp->prevp;
}


/* save the current macro as a named macro */

static COMPLETION(emxsavemacrocomplete)

{
    FUNCTION	*fp;
    int			maclen;
    int			i;

    if (status != TRUE)
	return status;

    for (maclen = 0; g_macro[maclen]; maclen += 2)
	;

    maclen++;

    if ((fp = findmacro(g_funcname)) == NULL)
	return FALSE;

    i = sizeof(unsigned long) * maclen;
    if ((fp->procp = (FUNC) malloc(i)) == 0)
	{
	removemacro(fp);
	return emxallocerr(i);
	}

    memcpy((char *)fp->procp, (char *)g_macro, maclen * sizeof(unsigned long));
    return TRUE;
}


COMMAND(emxsavemacro)

{
    if (!g_macro)
	{
	emxmsgprint("[No current macro]");
	return FALSE;
	}

    return emxmsgreply("Save macro as: ", g_funcname, sizeof(g_funcname),
		  emxsavemacrocomplete);
}


/* Make a macro by reading in keystrokes, without interpreting them, until we
   get a delimiter. */

static COMPLETION(emxmakemacrocomplete)

{
    if (g_macrolen < 0)
	{
	if (g_keyp->funcp == g_fabortp)
	    {
	    emxpromptoff();
	    emxabort();
	    removemacro(g_funcp);
	    return ABORT;
	    }

	g_macrodelim.keysym = g_keyp->keysym;
	g_macrodelim.modifier = g_keyp->modifier;
	g_macrolen = 0;
	g_macrokeys = (unsigned long *) malloc(MAXMACRO * sizeof(long));
	emxmsgput("keys: ");
	return emxonekey(emxmakemacrocomplete);
	}

    /* Got the delimiter - save the macro */
    if (g_keyp->keysym == g_macrodelim.keysym &&
	g_keyp->modifier == g_macrodelim.modifier)
	{
	g_macrokeys[g_macrolen] = 0;
	g_funcp->procp = (FUNC) realloc((char *)g_macrokeys,
					(g_macrolen + 1) * sizeof(long));
	emxpromptoff();

	/* See if the macro is to be bound */
	if (g_bindkey)
	    {
	    g_keyp = &g_keycurrent;
	    g_keyp->keysym = g_bindkey;
	    g_keyp->modifier = g_bindmod;
	    g_keyp = emxgetbinding(g_keyp);
	    return bindkey(g_keyp);
	    }

	return TRUE;
	}

    /* Got a key - store it and get another */
    if (g_keyp->funcp == g_fabortp)
	{
	emxpromptoff();
	emxabort();
	removemacro(g_funcp);
	return ABORT;
	}

    g_macrokeys[g_macrolen++] = g_keyp->keysym;
    g_macrokeys[g_macrolen++] = g_keyp->modifier;
    if (g_macrolen == MAXMACRO - 2)
	{
	emxpromptoff();
	removemacro(g_funcp);
	emxmsgprint("[Macro too long]");
	return FALSE;
	}

    return emxonekey(emxmakemacrocomplete);
}


static COMPLETION(emxmakemacropartial)

{
    if (status != TRUE)
	return status;

    if ((g_funcp = findmacro(g_funcname)) == NULL)
	return FALSE;

    g_macrolen = -1;
    g_bindkey = 0;
    emxprompton();
    emxmsgput(g_msgdelimiter);
    return emxonekey(emxmakemacrocomplete);
}


COMMAND(emxmakemacro)

{
    return emxmsgreply(g_msgnewmacro, g_funcname, sizeof(g_funcname),
		  emxmakemacropartial);
}


/* make-bound-macro: Make a new macro and bind it to a key */

static COMPLETION(emxmakeboundmacrokey)

{
    char    kname[MAXCMD];

    if (!g_initialload)
	{
	keyname(g_keyp, kname);		    /* Display keyname */
	emxmsgput(kname);
	}

    g_bindkey = g_keyp->keysym;
    g_bindmod = g_keyp->modifier;
    g_macrolen = -1;
    emxmsgput(g_msgdelimiter);
    return emxonekey(emxmakemacrocomplete);
}


static COMPLETION(emxmakeboundmacroname)

{
    if (status != TRUE)
	return status;

    if ((g_funcp = findmacro(g_funcname)) == NULL)
	return FALSE;

    emxprompton();
    emxmsgput(" key: ");
    return emxonekey(emxmakeboundmacrokey);
}


COMMAND(emxmakeboundmacro)

{
    return emxmsgreply(g_msgnewmacro, g_funcname, sizeof(g_funcname),
		  emxmakeboundmacroname);
}


COMMAND(emxcopyregiontomacro)

{
    FUNCTION	*fp;
    KEYBINDING	*bp;
    LINE	*linep;
    int		loffs;
    REGION	region;
    unsigned long    *macrop;

    if (g_macroinp || g_macrooutp)
	{
	emxmsgprint(g_msgnotnow);
	return FALSE;
	}

    /* Get the current region */
    if (!emxgetregion(&region))
	return FALSE;

    /* Get the newline function pointer */
    fp = funclookup(F_ins_nl);
    bp = fp->bindingp;

    /* Allocate space for the macro */
    g_macrolen = region.chars * 2 + 1;
    g_macrokeys = (unsigned long *) malloc(g_macrolen * sizeof(unsigned long));
    macrop = g_macrokeys;

    linep = region.firstlp;		    /* Current line */
    loffs = region.firsto;		    /* Current offset */

    while (region.chars)
	{
	/* End of line */
	if (loffs == linep->used)
	    {
	    *macrop++ = bp->keyp->keysym;
	    *macrop++ = bp->keyp->modifier;
	    linep = linep->nextp;
	    loffs = 0;
	    }

	else
	    {
	    *macrop++ = linep->text[loffs];
	    *macrop++ = 0;
	    loffs++;
	    }

	region.chars--;
	}

    *macrop = 0;
    if (g_macro)
	FREE(g_macro);

    g_macro = g_macrokeys;
    emxmsgprint(g_msgcopied);
    return TRUE;
}


/* Clear out the current key bindings and reset to the originals */

COMMAND0(emxoriginalbindings)

{
    FUNCTION    *fp;
    KEYBINDING  *bp;
    KEYBINDING  *nextbp;
    KEY		*kp;
    KEY		*nextkp;

    /* Clear the key bindings from the functions */
    for (fp = g_flistp; fp; fp = fp->nextp)
	{
	for (bp = fp->bindingp; bp; bp = nextbp)
	    {
	    nextbp = bp->nextp;
	    FREE(bp);
	    }

	fp->bindingp = NULL;
	}

    /* Free all the key structs */
    for (kp = g_keylistp; kp; kp = nextkp)
	{
	nextkp = kp->orderp;
	FREE(kp);
	}

    /* Reset to the originals */
    emxkeylistinit();
    return TRUE;
}


/* Clear out the current key bindings and reset to the user's originals */

COMMAND0(emxresetbindings)

{
    int   status;

    emxoriginalbindings();
    status = emxdefaultcustomization();
    if (status == TRUE)
	emxok();

    return status;
}


/* Clear out the current key bindings and reset to the specified user's */

static COMPLETION(emxloadbindingscomplete)

{
    char  buf[1024];

    if (status == ABORT)
	return status;

    if (g_cmdbuf[0] == 0)
	return emxresetbindings();

    /* Fetch the user's customization file, if available */
    emxoriginalbindings();

    strcpy(buf, "/home/");
    strcat(buf, g_cmdbuf);
    strcat(buf, "/.emxrc");
    status = emxplayfile(buf, TRUE);
    if (status == TRUE)
	emxok();

    return status;
}


COMMAND0(emxloadbindings)

{
    /* Prompt for a user name */
    return emxmsgreply("User name: ", g_cmdbuf, sizeof(g_cmdbuf),
		    emxloadbindingscomplete);
}


