" Vim syntax file
" Language:	Mkfile
" Maintainer:	Benjamin Stürz
" URL:		
" Last Change:	2025 May 27

" quit when a syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

" some special characters
syn match makeSpecial	"^\s*[@-]\+"
syn match makeNextLine	"\\\n\s*"

" .template
syn region makeDefine start="^\s*\.template\s" end="^\s*\.endt\(emplate\)\?"
	\ contains=makeStatement,makeIdent,makePreCondit,makeDefine

" identifiers
syn region makeIdent	start="\$(" skip="\\)\|\\\\" end=")" contains=makeStatement,makeIdent
syn region makeIdent	start="\${" skip="\\}\|\\\\" end="}" contains=makeStatement,makeIdent
syn match makeIdent	"\$\$\w*"
syn match makeIdent	"\$[^({]"
syn match makeIdent	"^ *[^:#= \t]*\s*[:+?!*]="me=e-2
syn match makeIdent	"^ *[^:#= \t]*\s*::="me=e-3
syn match makeIdent	"^ *[^:#= \t]*\s*="me=e-1
syn match makeIdent	"%"

" Makefile.in variables
syn match makeConfig "@[A-Za-z0-9_]\+@"

" make targets
syn match makeImplicit		"^\.[A-Za-z0-9_./\t -]\+\s*:$"me=e-1
syn match makeImplicit		"^\.[A-Za-z0-9_./\t -]\+\s*:[^=]"me=e-2

syn region makeTarget transparent matchgroup=makeTarget
	\ start="^[~A-Za-z0-9_./$(){}%-][A-Za-z0-9_./\t ${}()%-]*&\?:\?:\{1,2}[^:=]"rs=e-1
	\ end="[^\\]$"
	\ keepend contains=makeIdent,makeSpecTarget,makeNextLine,makeComment,makeDString
	\ skipnl nextGroup=makeCommands
syn match makeTarget           "^[~A-Za-z0-9_./$(){}%*@-][A-Za-z0-9_./\t $(){}%*@-]*&\?::\=\s*$"
	\ contains=makeIdent,makeSpecTarget,makeComment
	\ skipnl nextgroup=makeCommands,makeCommandError

syn region makeSpecTarget	transparent matchgroup=makeSpecTarget
	\ start="^\.\(DEFAULT\|SUFFIXES\|SUBDIRS\|FOREIGN\|EXPORTS\)\>\s*:\{1,2}[^:=]"rs=e-1
	\ end="[^\\]$" keepend
	\ contains=makeIdent,makeSpecTarget,makeNextLine,makeComment skipnl nextGroup=makeCommands
syn match makeSpecTarget	"^\.\(DEFAULT\|SUFFIXES\|SUBDIRS\|FOREIGN\|EXPORTS\)\>\s*::\=\s*$"
	\ contains=makeIdent,makeComment
	\ skipnl nextgroup=makeCommands,makeCommandError

syn match makeCommandError "^\s\+\S.*" contained
syn region makeCommands contained start=";"hs=s+1 start="^\t"
	\ end="^[^\t#]"me=e-1,re=e-1 end="^$"
	\ contains=makeCmdNextLine,makeSpecial,makeComment,makeIdent,makePreCondit,makeDefine,makeDString,makeSString
	\ nextgroup=makeCommandError
syn match makeCmdNextLine	"\\\n."he=e-1 contained

" some directives
syn match makePreCondit "^\.\s*\(if\|elif\|else\|endif\)\>.*$" contains=makeIfOps,makeIfString
syn match makeInclude	"^ *[-s]\=include\s.*$"
syn match makeInclude  "^ *\.\s*include"
syn keyword makeIfOps defined target contained
syn match makeIfString "\"[^\"]*\"" contains=makeIdent contained

" Comment
syn match   makeDocComment "##.*" contains=@Spell,makeTodo
syn match   makeComment	"#\([^#]\+.*\)\?$" contains=@Spell,makeTodo
syn keyword makeTodo TODO FIXME XXX contained

" match escaped quotes and any other escaped character
" except for $, as a backslash in front of a $ does
" not make it a standard character, but instead it will
" still act as the beginning of a variable
" The escaped char is not highlightet currently
syn match makeEscapedChar	"\\[^$]"


syn region  makeDString start=+\(\\\)\@<!"+  skip=+\\.+  end=+"+  contained contains=makeIdent
syn region  makeSString start=+\(\\\)\@<!'+  skip=+\\.+  end=+'+  contained contains=makeIdent
syn region  makeBString start=+\(\\\)\@<!`+  skip=+\\.+  end=+`+  contains=makeIdent,makeSString,makeDString,makeNextLine

" Syncing
syn sync minlines=20 maxlines=200

" Sync on Make command block region: When searching backwards hits a line that
" can't be a command or a comment, use makeCommands if it looks like a target,
" NONE otherwise.
syn sync match makeCommandSync groupthere NONE "^[^\t#]"
syn sync match makeCommandSync groupthere makeCommands "^[A-Za-z0-9_./$()%-][A-Za-z0-9_./\t $()%-]*:\{1,2}[^:=]"
syn sync match makeCommandSync groupthere makeCommands "^[A-Za-z0-9_./$()%-][A-Za-z0-9_./\t $()%-]*:\{1,2}\s*$"

" Define the default highlighting.
" Only when an item doesn't have highlighting yet

hi def link makeNextLine	makeSpecial
hi def link makeCmdNextLine	makeSpecial
hi link     makeOverride        makeStatement
hi link     makeExport          makeStatement

hi def link makeSpecTarget	Statement
if !exists("make_no_commands")
hi def link makeCommands	Number
endif
hi def link makeImplicit	Function
hi def link makeTarget		Function
hi def link makeInclude		Include
hi def link makePreCondit	PreCondit
hi def link makeStatement	Statement
hi def link makeIdent		Identifier
hi def link makeSpecial		Special
hi def link makeComment		Comment
hi def link makeDocComment	SpecialComment
hi def link makeDString		String
hi def link makeSString		String
hi def link makeBString		Function
hi def link makeError		Error
hi def link makeTodo		Todo
hi def link makeDefine		Define
hi def link makeCommandError	Error
hi def link makeConfig		PreCondit
hi def link makeIfOps		Function
hi def link makeIfString	String

let b:current_syntax = "bmk"

let &cpo = s:cpo_save
unlet s:cpo_save
" vim: ts=8
