/** Event based wrapper for various tasks on the "Save in Arcollect" button
 *
 * This object manage a part of the Arcollect saving logic and allow you to
 * write more high-level callbacks, this makes your content-script more future
 * proof and robust.
 */
Arcollect.SaveControlHelper = class {
	/** The root "Save in Arcollect" button element
	 *
	 * \note It is the first param of SaveControlHelper(), not in it's config.
	 */
	SaveInArcollectElements;
	/** Artwork generation
	 * \return A promise with the JSON object to pass to Arcollect.submit().
	 * \note It is the second param of SaveControlHelper(), not in it's config.
	 */
	onMakeWebextAdderPayload;
	
	/** Element of the button label
	 *
	 * The label part of the "Save in Arcollect" button, it's "contentText" is
	 * managed by this object. It is required.
	 */
	ButtonLabelElements;
	/** Called when saving begins
	 * \return An empty fullfilled promise
	 * \note This is an asynchronous function.
	 */
	onSaveBegin;
	/** Called when saving succeed
	 */
	onSaveSuccess;
	/** Called when saving fails
	 * \param reason of failure
	 */
	onSaveFailure;
	
	saveInArcollectClicked() {
		this.onSaveBegin().then(this.onMakeWebextAdderPayload).then(Arcollect.submit).then(this.onSaveSuccess).catch(this.onSaveFailure);
	}
	saveInArcollectClickedEventHandler() {
		this.ArcollectSaveControlHelper.saveInArcollectClicked();
	}
	
	/** Constructor
	 * \param SaveInArcollectElements iterable that enclose the "Save in Arcollect" button
	 * \param onMakeWebextAdderPayload that generate the payload.
	 * \param config to use. Please read each field documentation individually.
	 */
	constructor(SaveInArcollectElements, onMakeWebextAdderPayload, config = {}) {
		// Configure SaveInArcollectElements
		if (typeof SaveInArcollectElements[Symbol.iterator] !== 'function')
			SaveInArcollectElements = [SaveInArcollectElements];
		this.SaveInArcollectElements = SaveInArcollectElements;
		for (const SaveInArcollectElement of SaveInArcollectElements) {
			SaveInArcollectElement.ArcollectSaveControlHelper = this;
			SaveInArcollectElement.addEventListener('click',this.saveInArcollectClickedEventHandler,{'passive': true});
			SaveInArcollectElement.style.cursor = "pointer";
		}
		
		this.ButtonLabelElements = config.ButtonLabelElements ? config.ButtonLabelElements : SaveInArcollectElements;
		for (const ButtonLabelElement of this.ButtonLabelElements)
			ButtonLabelElement.textContent = browser.i18n.getMessage('webext_save_in_arcollect');
		
		this.onSaveBegin = async () => {
			for (const ButtonLabelElement of this.ButtonLabelElements)
				ButtonLabelElement.textContent = browser.i18n.getMessage('webext_saving_in_arcollect');
			
			for (const SaveInArcollectElement of this.SaveInArcollectElements) {
				SaveInArcollectElement.style.cursor = "progress";
				SaveInArcollectElement.removeEventListener('click',this.saveInArcollectClickedEventHandler,{'passive': true});
			}
			if (config.onSaveBegin)
				return await config.onSaveBegin();
		}
		
		this.onSaveSuccess = async () => {
			// Update the UI
			for (const ButtonLabelElement of this.ButtonLabelElements) {
				ButtonLabelElement.textContent = browser.i18n.getMessage('webext_saved_in_arcollect');
			}
			for (const SaveInArcollectElement of this.SaveInArcollectElements) {
				SaveInArcollectElement.style.cursor = "default";
			}
			
			// Callback
			if (config.onSaveSuccess)
				return config.onSaveSuccess();
		};
		
		this.onSaveFailure = async (reason) => {
			console.error("Failed to save in Arcollect!",reason);
			alert(arco_i18n_save_fail+' '+reason);
			for (const ButtonLabelElement of this.ButtonLabelElements) {
				ButtonLabelElement.textContent = browser.i18n.getMessage('webext_save_in_arcollect_retry');
			}
			for (const SaveInArcollectElement of this.SaveInArcollectElements) {
				SaveInArcollectElement.addEventListener('click',this.saveInArcollectClickedEventHandler,{'passive': true});
				SaveInArcollectElement.style.cursor = "pointer";
			}
			if (config.onSaveFailure)
				return config.onSaveFailure();
		};
		
		this.onMakeWebextAdderPayload = onMakeWebextAdderPayload;
		if (!(onMakeWebextAdderPayload instanceof Function))
			throw "Incorrect onMakeWebextAdderPayload";
	};
}
