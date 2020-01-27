function showMenu() {
    if (navMenu.style.display == "block") {
        navMenu.style.display = "none";
        search.style.display = "none";
        navButton.innerHTML = navButton.innerHTML.slice(0, navButton.innerHTML.indexOf("Меню")) + active.innerHTML.slice(active.innerHTML.indexOf("</i>")+4, active.innerHTML.indexOf("</a>"));
    }
    else {
        navMenu.style.display = "block";
        search.style.display = "block";
        navButton.innerHTML = navButton.innerHTML.slice(0, navButton.innerHTML.indexOf("</button")+9)+"Меню";
    }
}
window.onload = function ready(){
    navButton.innerHTML = navButton.innerHTML.slice(0, navButton.innerHTML.indexOf("Меню")) + active.innerHTML.slice(active.innerHTML.indexOf("</i>")+4, active.innerHTML.indexOf("</a>"));
};