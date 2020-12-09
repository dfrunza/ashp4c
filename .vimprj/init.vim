" See the article http://dmitryfrank.com/articles/vim_project_code_navigation

" Get path to ".vimprj" folder
let s:sPath = expand('<sfile>:p:h')

let g:indexer_ctagsCommandLineOptions="--fields=+iaSl --C-kinds=+p --C++-kinds=+p --extras=+q --languages=C,C++"

" Specify the project's 'indexer_files'"
let g:indexer_indexerListFilename = s:sPath.'/indexer_files'

set errorformat=
set errorformat+=%E%f:%l:%c:\ %t\rror:\ %m,%-C,%-Z%p^
set errorformat+=%f:%l:%c:\ fatal\ %t\rror:\ %m,%-C,%-Z%p^
set errorformat+=%f:%l:%c:\ %t\arning:\ %m,%-C,%-Z%p^
set errorformat+=%D%*\\a:\ Entering\ directory\ [`']%f'
set errorformat+=%X%*\\a:\ Leaving\ directory\ [`']%f'

" Linker error 'undefined reference'
set errorformat+=%f:%l:\ %m,%-C,%-Z%p^
set errorformat+=collect2:\ %t\rror:\ ld\ returned\ 1\ exit\ status

set shiftwidth=2 " indendation level
set tabstop=2
set cindent
