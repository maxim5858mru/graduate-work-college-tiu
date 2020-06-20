// Реализация переключение мобильного меню
function showMenu() {
    if (nav.style.display == "block") {
        nav.style.display = "none";
        navbutton.innerHTML = navbutton.innerHTML.slice(0, navbutton.innerHTML.indexOf("Меню")) + active.innerHTML.slice(active.innerHTML.indexOf("</i>")+4, active.innerHTML.indexOf("</a>"));
    }
    else {
        nav.style.display = "block";
        navbutton.innerHTML = navbutton.innerHTML.slice(0, navbutton.innerHTML.indexOf("</button")+9)+"Меню";
    }
}

// Установка в мобильной версии заголовка текущего раздела
window.onload = function ready() {
    navbutton.innerHTML = navbutton.innerHTML.slice(0, navbutton.innerHTML.indexOf("Меню")) + active.innerHTML.slice(active.innerHTML.indexOf("</i>")+4, active.innerHTML.indexOf("</a>"));
};

// Нормализация работы меню при перевороте экрана
window.addEventListener('resize', function(){
    if (document.body.clientWidth > 600) {
        nav.style.display = "block";
        navbutton.innerHTML = navbutton.innerHTML.slice(0, navbutton.innerHTML.indexOf("</button")+9)+"Меню";
    }
    else if (nav.style.display == "block") {
        showMenu();
    }
});