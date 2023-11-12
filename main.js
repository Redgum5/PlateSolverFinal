window.addEventListener("DOMContentLoaded", () => {
	document.getElementById("close-nav-left").addEventListener("click", () => {
		const left_nav_element = document.getElementsByClassName("nav-side")[0];
		const nav_cover_element = document.getElementsByClassName("nav-cover")[0];
		left_nav_element.classList.add("navbar-hidden");
		nav_cover_element.classList.add("navbar-cover-hidden");

	})
	document.getElementById("close-nav-left-cover").addEventListener("click", () => {
		const left_nav_element = document.getElementsByClassName("nav-side")[0];
		const nav_cover_element = document.getElementsByClassName("nav-cover")[0];
		left_nav_element.classList.add("navbar-hidden");
		nav_cover_element.classList.add("navbar-cover-hidden");

	})
	document.getElementById("open-nav-left").addEventListener("click", () => {
		const left_nav_element = document.getElementsByClassName("nav-side")[0];
		const nav_cover_element = document.getElementsByClassName("nav-cover")[0];
		left_nav_element.classList.remove("navbar-hidden");
		nav_cover_element.classList.remove("navbar-cover-hidden");
	})
	try
	{
		document.getElementById("fileToUpload").addEventListener("change", () => {
			const span_element = document.getElementById("file-selected");
			const input_element = document.getElementById("fileToUpload");

			var fileName = input_element.value;

			console.log(fileName);

			if (fileName != "") {
				span_element.classList.remove("input-hidden");
				input_element.classList.add("input-hidden");
			}
			else {
				span_element.classList.add("input-hidden");
				input_element.classList.remove("input-hidden");
			}

			span_element.textContent = fileName.split("\\").pop();
			
			
		})
	}
	catch (e)
	{
		console.log("elementNotFound");
		console.log("Error", e.stack);
	}
	try
	{

		document.getElementById("toggle-advanced").addEventListener("click", () => {
			const advanced_element = document.getElementsByClassName("solve-advanced")[0];
			const open_symbol = document.getElementById("advanced-open");
			const close_symbol = document.getElementById("advanced-close");

			advanced_element.classList.toggle("solve-advanced-hidden");
			open_symbol.classList.toggle("icon-hidden");
			close_symbol.classList.toggle("icon-hidden");
		})
	}
	catch (e)
	{
		console.log("elementNotFound");
		console.log("Error", e.stack);
	}
	try
	{
		document.getElementById("php-dropdown-button-top").addEventListener("click", () => {
			const dropdown_element = document.getElementById("php-dropdown-top");
			const open_symbol = document.getElementById("php-dropdown-button-top-open");
			const close_symbol = document.getElementById("php-dropdown-button-top-close");
			console.log("click");

			dropdown_element.classList.toggle("php-dropdown-hidden");
			open_symbol.classList.toggle("php-icon-hidden");
			close_symbol.classList.toggle("php-icon-hidden");
		})
		document.getElementById("php-dropdown-button-bottom").addEventListener("click", () => {
			const dropdown_element = document.getElementById("php-dropdown-bottom");
			const open_symbol = document.getElementById("php-dropdown-button-bottom-open");
			const close_symbol = document.getElementById("php-dropdown-button-bottom-close");
			console.log("click");

			dropdown_element.classList.toggle("php-dropdown-hidden");
			open_symbol.classList.toggle("php-icon-hidden");
			close_symbol.classList.toggle("php-icon-hidden");
		})
	}
	catch (e)
	{
		console.log("elementNotFound");
		console.log("Error", e.stack);
	}
	
})