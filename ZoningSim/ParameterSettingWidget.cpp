#include "ParameterSettingWidget.h"

ParameterSettingWidget::ParameterSettingWidget(QWidget* parent, QMap<QString, float>& parameters) : QDialog((QWidget*)parent), parameters(parameters) {
	// set up the UI
	ui.setupUi(this);

	ui.lineEditHighwayAccessibility->setText(QString::number(parameters["highway_accessibility"]));
	ui.lineEditAvenueAccessibility->setText(QString::number(parameters["avenue_accessibility"]));
	ui.lineEditStreetAccessibility->setText(QString::number(parameters["street_accessibility"]));

	ui.lineEditPopulationNeighbor->setText(QString::number(parameters["population_neighbor"]));
	ui.lineEditDistanceNeighborPopulation->setText(QString::number(parameters["distance_neighbor_population"]));
	ui.lineEditCommercialNeighbor->setText(QString::number(parameters["commercial_neighbor"]));
	ui.lineEditDistanceNeighborCommercial->setText(QString::number(parameters["distance_neighbor_commercial"]));
	ui.lineEditIndustrialPollution->setText(QString::number(parameters["industrial_pollution"]));
	ui.lineEditDistancePollution->setText(QString::number(parameters["distance_pollution"]));

	ui.lineEditAccessibilityLandValue->setText(QString::number(parameters["accessibility_landvalue"]));
	ui.lineEditNeighborPopulationLandValue->setText(QString::number(parameters["neighbor_population_landvalue"]));
	ui.lineEditNeighborCommercialLandValue->setText(QString::number(parameters["neighbor_commercial_landvalue"]));
	ui.lineEditPollutionLandValue->setText(QString::number(parameters["pollution_landvalue"]));
	ui.lineEditSlopeLandValue->setText(QString::number(parameters["slope_landvalue"]));
	ui.lineEditPopulationLandValue->setText(QString::number(parameters["population_landvalue"]));
	ui.lineEditCommercialJobsLandValue->setText(QString::number(parameters["commercialjobs_landvalue"]));
	ui.lineEditIndustrialJobsLandValue->setText(QString::number(parameters["industrialjobs_landvalue"]));

	ui.lineEditAccessibilityLife->setText(QString::number(parameters["accessibility_life"]));
	ui.lineEditNeighborPopulationLife->setText(QString::number(parameters["neighbor_population_life"]));
	ui.lineEditNeighborCommercialLife->setText(QString::number(parameters["neighbor_commercial_life"]));
	ui.lineEditPollutionLife->setText(QString::number(parameters["pollution_life"]));
	ui.lineEditSlopeLife->setText(QString::number(parameters["slope_life"]));
	ui.lineEditLandValueLife->setText(QString::number(parameters["landvalue_life"]));
	ui.lineEditPopulationLife->setText(QString::number(parameters["population_life"]));
	ui.lineEditCommercialJobsLife->setText(QString::number(parameters["commercialjobs_life"]));
	ui.lineEditIndustrialJobsLife->setText(QString::number(parameters["industrialjobs_life"]));

	ui.lineEditAccessibilityShop->setText(QString::number(parameters["accessibility_shop"]));
	ui.lineEditNeighborPopulationShop->setText(QString::number(parameters["neighbor_population_shop"]));
	ui.lineEditNeighborCommercialShop->setText(QString::number(parameters["neighbor_commercial_shop"]));
	ui.lineEditPollutionShop->setText(QString::number(parameters["pollution_shop"]));
	ui.lineEditSlopeShop->setText(QString::number(parameters["slope_shop"]));
	ui.lineEditLandValueShop->setText(QString::number(parameters["landvalue_shop"]));
	ui.lineEditPopulationShop->setText(QString::number(parameters["population_shop"]));
	ui.lineEditCommercialJobsShop->setText(QString::number(parameters["commercialjobs_shop"]));
	ui.lineEditIndustrialJobsShop->setText(QString::number(parameters["industrialjobs_shop"]));

	ui.lineEditAccessibilityFactory->setText(QString::number(parameters["accessibility_factory"]));
	ui.lineEditNeighborPopulationFactory->setText(QString::number(parameters["neighbor_population_factory"]));
	ui.lineEditNeighborCommercialFactory->setText(QString::number(parameters["neighbor_commercial_factory"]));
	ui.lineEditPollutionFactory->setText(QString::number(parameters["pollution_factory"]));
	ui.lineEditSlopeFactory->setText(QString::number(parameters["slope_factory"]));
	ui.lineEditLandValueFactory->setText(QString::number(parameters["landvalue_factory"]));
	ui.lineEditPopulationFactory->setText(QString::number(parameters["population_factory"]));
	ui.lineEditCommercialJobsFactory->setText(QString::number(parameters["commercialjobs_factory"]));
	ui.lineEditIndustrialJobsFactory->setText(QString::number(parameters["industrialjobs_factory"]));

	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void ParameterSettingWidget::onOK() {
	parameters["highway_accessibility"] = ui.lineEditHighwayAccessibility->text().toFloat();
	parameters["avenue_accessibility"] = ui.lineEditAvenueAccessibility->text().toFloat();
	parameters["street_accessibility"] = ui.lineEditStreetAccessibility->text().toFloat();

	parameters["population_neighbor"] = ui.lineEditPopulationNeighbor->text().toFloat();
	parameters["distance_neighbor_population"] = ui.lineEditDistanceNeighborPopulation->text().toFloat();
	parameters["commercial_neighbor"] = ui.lineEditCommercialNeighbor->text().toFloat();
	parameters["distance_neighbor_commercial"] = ui.lineEditDistanceNeighborCommercial->text().toFloat();
	parameters["industrial_pollution"] = ui.lineEditIndustrialPollution->text().toFloat();
	parameters["distance_pollution"] = ui.lineEditDistancePollution->text().toFloat();

	parameters["accessibility_landvalue"] = ui.lineEditAccessibilityLandValue->text().toFloat();
	parameters["neighbor_population_landvalue"] = ui.lineEditNeighborPopulationLandValue->text().toFloat();
	parameters["neighbor_commercial_landvalue"] = ui.lineEditNeighborCommercialLandValue->text().toFloat();
	parameters["pollution_landvalue"] = ui.lineEditPollutionLandValue->text().toFloat();
	parameters["slope_landvalue"] = ui.lineEditSlopeLandValue->text().toFloat();
	parameters["population_landvalue"] = ui.lineEditPopulationLandValue->text().toFloat();
	parameters["commercialjobs_landvalue"] = ui.lineEditCommercialJobsLandValue->text().toFloat();
	parameters["industrialjobs_landvalue"] = ui.lineEditIndustrialJobsLandValue->text().toFloat();

	parameters["accessibility_life"] = ui.lineEditAccessibilityLife->text().toFloat();
	parameters["neighbor_population_life"] = ui.lineEditNeighborPopulationLife->text().toFloat();
	parameters["neighbor_commercial_life"] = ui.lineEditNeighborCommercialLife->text().toFloat();
	parameters["pollution_life"] = ui.lineEditPollutionLife->text().toFloat();
	parameters["slope_life"] = ui.lineEditSlopeLife->text().toFloat();
	parameters["landvalue_life"] = ui.lineEditLandValueLife->text().toFloat();
	parameters["population_life"] = ui.lineEditPopulationLife->text().toFloat();
	parameters["commercialjobs_life"] = ui.lineEditCommercialJobsLife->text().toFloat();
	parameters["industrialjobs_life"] = ui.lineEditIndustrialJobsLife->text().toFloat();

	parameters["accessibility_shop"] = ui.lineEditAccessibilityShop->text().toFloat();
	parameters["neighbor_population_shop"] = ui.lineEditNeighborPopulationShop->text().toFloat();
	parameters["neighbor_commercial_shop"] = ui.lineEditNeighborCommercialShop->text().toFloat();
	parameters["pollution_shop"] = ui.lineEditPollutionShop->text().toFloat();
	parameters["slope_shop"] = ui.lineEditSlopeShop->text().toFloat();
	parameters["landvalue_shop"] = ui.lineEditLandValueShop->text().toFloat();
	parameters["population_shop"] = ui.lineEditPopulationShop->text().toFloat();
	parameters["commercialjobs_shop"] = ui.lineEditCommercialJobsShop->text().toFloat();
	parameters["industrialjobs_shop"] = ui.lineEditIndustrialJobsShop->text().toFloat();

	parameters["accessibility_factory"] = ui.lineEditAccessibilityFactory->text().toFloat();
	parameters["neighbor_population_factory"] = ui.lineEditNeighborPopulationFactory->text().toFloat();
	parameters["neighbor_commercial_factory"] = ui.lineEditNeighborCommercialFactory->text().toFloat();
	parameters["pollution_factory"] = ui.lineEditPollutionFactory->text().toFloat();
	parameters["slope_factory"] = ui.lineEditSlopeFactory->text().toFloat();
	parameters["landvalue_factory"] = ui.lineEditLandValueFactory->text().toFloat();
	parameters["population_factory"] = ui.lineEditPopulationFactory->text().toFloat();
	parameters["commercialjobs_factory"] = ui.lineEditCommercialJobsFactory->text().toFloat();
	parameters["industrialjobs_factory"] = ui.lineEditIndustrialJobsFactory->text().toFloat();
	
	this->accept();
}
