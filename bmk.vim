" Vim syntax file
" Language:	Mkfile
" Maintainer:	Benjamin Stürz
" URL:		https://github.com/realchonk/bmk
" Last Change:	2025 May 27

" quit when a syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

" some special characters
syn match bmkSpecial	"^\s*[@-]\+"
syn match bmkNextLine	"\\\n\s*"

" .template
syn region bmkDefine start="^\s*\.template\s" end="^\s*\.endt\(emplate\)\?"
	\ contains=bmkStatement,bmkIdent,bmkPreCondit,bmkDefine

" identifiers
syn region bmkIdent	start="\$(" skip="\\)\|\\\\" end=")" contains=bmkStatement,bmkIdent
syn region bmkIdent	start="\${" skip="\\}\|\\\\" end="}" contains=bmkStatement,bmkIdent
syn match bmkIdent	"\$\$\w*"
syn match bmkIdent	"\$[^({]"
syn match bmkIdent	"^ *[^:#= \t]*\s*[:+?!*]="me=e-2
syn match bmkIdent	"^ *[^:#= \t]*\s*::="me=e-3
syn match bmkIdent	"^ *[^:#= \t]*\s*="me=e-1
syn match bmkIdent	"%"

" Makefile.in variables
syn match bmkConfig "@[A-Za-z0-9_]\+@"

" bmk targets
syn match bmkImplicit		"^\.[A-Za-z0-9_./\t -]\+\s*:$"me=e-1
syn match bmkImplicit		"^\.[A-Za-z0-9_./\t -]\+\s*:[^=]"me=e-2

syn region bmkTarget transparent matchgroup=bmkTarget
	\ start="^[~A-Za-z0-9_./$(){}%-][A-Za-z0-9_./\t ${}()%-]*&\?:\?:\{1,2}[^:=]"rs=e-1
	\ end="[^\\]$"
	\ keepend contains=bmkIdent,bmkSpecTarget,bmkNextLine,bmkComment,bmkDString
	\ skipnl nextGroup=bmkCommands
syn match bmkTarget           "^[~A-Za-z0-9_./$(){}%*@-][A-Za-z0-9_./\t $(){}%*@-]*&\?::\=\s*$"
	\ contains=bmkIdent,bmkSpecTarget,bmkComment
	\ skipnl nextgroup=bmkCommands,bmkCommandError

syn region bmkSpecTarget	transparent matchgroup=bmkSpecTarget
	\ start="^\.\(DEFAULT\|SUFFIXES\|SUBDIRS\|FOREIGN\|EXPORTS\)\>\s*:\{1,2}[^:=]"rs=e-1
	\ end="[^\\]$" keepend
	\ contains=bmkIdent,bmkSpecTarget,bmkNextLine,bmkComment skipnl nextGroup=bmkCommands
syn match bmkSpecTarget	"^\.\(DEFAULT\|SUFFIXES\|SUBDIRS\|FOREIGN\|EXPORTS\)\>\s*::\=\s*$"
	\ contains=bmkIdent,bmkComment
	\ skipnl nextgroup=bmkCommands,bmkCommandError

syn match bmkCommandError "^\s\+\S.*" contained
syn region bmkCommands contained start=";"hs=s+1 start="^\t"
	\ end="^[^\t#]"me=e-1,re=e-1 end="^$"
	\ contains=bmkCmdNextLine,bmkSpecial,bmkComment,bmkIdent,bmkPreCondit,bmkDefine,bmkDString,bmkSString
	\ nextgroup=bmkCommandError
syn match bmkCmdNextLine	"\\\n."he=e-1 contained

" some directives
syn match bmkPreCondit "^\.\s*\(if\|elif\|else\|endif\)\>.*$" contains=bmkIfOps,bmkIfString
syn match bmkInclude	"^ *[-s]\=include\s.*$"
syn match bmkInclude  "^ *\.\s*include"
syn match bmkInclude  "^ *\.\s*expand"
syn keyword bmkIfOps defined target contained
syn match bmkIfString "\"[^\"]*\"" contains=bmkIdent contained

" Comment
syn match   bmkDocComment "##.*" contains=@Spell,bmkTodo
syn match   bmkComment	"#\([^#]\+.*\)\?$" contains=@Spell,bmkTodo
syn keyword bmkTodo TODO FIXME XXX contained

" match escaped quotes and any other escaped character
" except for $, as a backslash in front of a $ does
" not bmk it a standard character, but instead it will
" still act as the beginning of a variable
" The escaped char is not highlightet currently
syn match bmkEscapedChar	"\\[^$]"


syn region  bmkDString start=+\(\\\)\@<!"+  skip=+\\.+  end=+"+  contained contains=bmkIdent
syn region  bmkSString start=+\(\\\)\@<!'+  skip=+\\.+  end=+'+  contained contains=bmkIdent
syn region  bmkBString start=+\(\\\)\@<!`+  skip=+\\.+  end=+`+  contains=bmkIdent,bmkSString,bmkDString,bmkNextLine

" Syncing
syn sync minlines=20 maxlines=200

" Sync on Make command block region: When searching backwards hits a line that
" can't be a command or a comment, use bmkCommands if it looks like a target,
" NONE otherwise.
syn sync match bmkCommandSync groupthere NONE "^[^\t#]"
syn sync match bmkCommandSync groupthere bmkCommands "^[A-Za-z0-9_./$()%-][A-Za-z0-9_./\t $()%-]*:\{1,2}[^:=]"
syn sync match bmkCommandSync groupthere bmkCommands "^[A-Za-z0-9_./$()%-][A-Za-z0-9_./\t $()%-]*:\{1,2}\s*$"

" Define the default highlighting.
" Only when an item doesn't have highlighting yet

hi def link bmkNextLine	bmkSpecial
hi def link bmkCmdNextLine	bmkSpecial
hi link     bmkOverride        bmkStatement
hi link     bmkExport          bmkStatement

hi def link bmkSpecTarget	Statement
if !exists("bmk_no_commands")
hi def link bmkCommands	Number
endif
hi def link bmkImplicit	Function
hi def link bmkTarget		Function
hi def link bmkInclude		Include
hi def link bmkPreCondit	PreCondit
hi def link bmkStatement	Statement
hi def link bmkIdent		Identifier
hi def link bmkSpecial		Special
hi def link bmkComment		Comment
hi def link bmkDocComment	SpecialComment
hi def link bmkDString		String
hi def link bmkSString		String
hi def link bmkBString		Function
hi def link bmkError		Error
hi def link bmkTodo		Todo
hi def link bmkDefine		Define
hi def link bmkCommandError	Error
hi def link bmkConfig		PreCondit
hi def link bmkIfOps		Function
hi def link bmkIfString	String

let b:current_syntax = "bmk"

let &cpo = s:cpo_save
unlet s:cpo_save
" vim: ts=8
