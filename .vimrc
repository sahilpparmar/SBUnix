inoremap jj <ESC> 
set nocompatible
set number
set autoindent
set smartindent
set shiftwidth=4 softtabstop=4
set incsearch ignorecase hlsearch
set showmatch
filetype plugin on
set ft=cpp
set smartcase
set smarttab
set cul
set vb t_vb=".
set noswapfile
set nobackup
set expandtab
set backspace+=indent,eol,start
set whichwrap+=l,h
set nowrap
set hidden
nnoremap <silent> <Space> :nohlsearch<Bar>:echo<CR>
nnoremap Y y$
set clipboard=unnamed
set path+=/home/stufs1/spparmar/cse506/**
syntax on

" Save and restore folding of lines
autocmd BufWinLeave *.* mkview
autocmd BufWinEnter *.* silent loadview

" Generate ctags in required dir and switch back to current directory
if has("win32") || has("win64")
    let g:tagsDir = 'c:/tmp'
else
    let g:tagsDir = '~/ctags'
endif

function! CtagsOverPath()
    let l:curdir = getcwd()
    execute 'silent cd ' . g:tagsDir
    execute 'silent !ctags -R ~/cse506' 
    execute 'silent cd ' . l:curdir
endfunction

execute 'silent set tags=' . g:tagsDir . '/tags'
command! GenerateCtags call CtagsOverPath()
nnoremap <silent> <F9> :GenerateCtags<CR>


